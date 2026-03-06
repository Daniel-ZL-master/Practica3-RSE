#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>
#include "esp_log.h"

static const char *TAG = "NTP_CONTROL_APP"; 

const char* ssid = "Redmi Note 14 5G";
const char* password = "daniTestAp";

const char* server_ip = "10.68.114.211";
const uint16_t server_port = 8080;

WiFiClient client;

const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "europe.pool.ntp.org";
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3"; 

// --- Variables de control de estado ---
bool isSending = false;
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 1000;

void timeSyncCallback(struct timeval *t) {
  ESP_LOGI(TAG, "¡Sincronización de hora disparada por el callback!");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  ESP_LOGI(TAG, "Iniciando conexión a la red WiFi: %s", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  ESP_LOGI(TAG, "WiFi Conectado. IP local: %s", WiFi.localIP().toString().c_str());

  sntp_set_time_sync_notification_cb(timeSyncCallback);

  ESP_LOGI(TAG, "Configurando e inicializando cliente NTP...");
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, ntpServer1);
  esp_sntp_setservername(1, ntpServer2);
  esp_sntp_init();

  setenv("TZ", time_zone, 1);
  tzset();

  ESP_LOGI(TAG, "Esperando la hora correcta desde los servidores...");
  int retry = 0;
  const int retry_count = 15;
  
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      delay(2000);
  }
  
  if (retry == retry_count) {
      ESP_LOGE(TAG, "No se pudo sincronizar la hora.");
  } else {
      ESP_LOGI(TAG, "Reloj del sistema actualizado correctamente.");
  }
}

void loop() {
  // 1. Gestión de la conexión TCP
  if (!client.connected()) {
    isSending = false; // Si se desconecta, paramos de enviar por seguridad
    ESP_LOGW(TAG, "Socket desconectado. Intentando conectar a %s:%d...", server_ip, server_port);
    
    if (client.connect(server_ip, server_port)) {
      ESP_LOGI(TAG, "¡Conectado al servidor! Esperando comando 'start'...");
    } else {
      ESP_LOGE(TAG, "Fallo al conectar. Reintentando en 3 segundos...");
      delay(3000); // Aquí sí podemos usar delay porque no hay conexión que atender
      return; 
    }
  }

  // 2. Leer comandos entrantes desde el PC
  while (client.available()) {
    String command = client.readStringUntil('\n');
    command.trim(); // Limpiamos espacios o saltos de línea extra (\r)

    if (command.equalsIgnoreCase("start")) {
      isSending = true;
      ESP_LOGI(TAG, "Comando START recibido. Iniciando transmisión.");
    } 
    else if (command.equalsIgnoreCase("stop")) {
      isSending = false;
      ESP_LOGI(TAG, "Comando STOP recibido. Transmisión pausada.");
    }
  }

  // 3. Enviar la hora cada 1 segundo
  if (isSending && (millis() - lastSendTime >= SEND_INTERVAL)) {
    lastSendTime = millis();

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      ESP_LOGW(TAG, "Fallo al leer el reloj interno (RTC).");
    } else {
      char timeStringBuff[50];
      strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
      
      client.println(timeStringBuff); // Enviamos al PC
      ESP_LOGI(TAG, "Enviando: %s", timeStringBuff); // Log local
    }
  }

  delay(10); // evitar saturación
}