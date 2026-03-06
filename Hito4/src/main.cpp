#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>
#include <esp_log.h>

static const char *TAG = "NTP_SOCKET_APP"; 

// ---------------------------------------------------------
// Configuración de red WiFi
// ---------------------------------------------------------
const char* ssid       = "Redmi Note 14 5G";
const char* password   = "daniTestAp";

// ---------------------------------------------------------
// Configuración del Socket TCP 
// ---------------------------------------------------------
const char* server_ip  = "10.68.114.211"; 
const uint16_t server_port = 8080;        

WiFiClient client;

// ---------------------------------------------------------
// Configuración NTP y Zona Horaria
// ---------------------------------------------------------
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "europe.pool.ntp.org";
const char* time_zone  = "CET-1CEST,M3.5.0,M10.5.0/3"; 

void timeSyncCallback(struct timeval *t) {
  ESP_LOGI(TAG, "¡Sincronización de hora disparada por el callback!");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // 1. Conexión WiFi
  ESP_LOGI(TAG, "Iniciando conexión a la red WiFi: %s", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  ESP_LOGI(TAG, "WiFi Conectado. IP local: %s", WiFi.localIP().toString().c_str());

  // 2. Configurar el callback nativo
  sntp_set_time_sync_notification_cb(timeSyncCallback);

  // 3. Inicializar SNTP (Actualizado para el Core 3.x)
  ESP_LOGI(TAG, "Configurando e inicializando cliente NTP...");
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, ntpServer1);
  esp_sntp_setservername(1, ntpServer2);
  esp_sntp_init();

  // 4. Configurar la zona horaria
  setenv("TZ", time_zone, 1);
  tzset();

  // 5. Esperar la sincronización (Actualizado para el Core 3.x)
  ESP_LOGI(TAG, "Esperando la hora correcta desde los servidores...");
  int retry = 0;
  const int retry_count = 15;
  
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      ESP_LOGI(TAG, "Sincronizando... (Intento %d/%d)", retry, retry_count);
      delay(2000);
  }
  
  if (retry == retry_count) {
      ESP_LOGE(TAG, "No se pudo sincronizar la hora.");
  } else {
      ESP_LOGI(TAG, "Reloj del sistema actualizado correctamente.");
  }
}

void loop() {
  struct tm timeinfo;
  
  if (!getLocalTime(&timeinfo)) {
    ESP_LOGW(TAG, "Fallo al leer el reloj interno (RTC).");
  } else {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    // Imprimimos en el monitor serie local
    ESP_LOGI(TAG, "Hora actual: %s", timeStringBuff);

    // Gestión del socket TCP
    if (!client.connected()) {
      ESP_LOGW(TAG, "Socket desconectado. Intentando conectar a %s:%d...", server_ip, server_port);
      if (client.connect(server_ip, server_port)) {
        ESP_LOGI(TAG, "¡Conectado al servidor en el PC!");
      } else {
        ESP_LOGE(TAG, "Fallo al conectar al PC. Reintentando en el próximo ciclo...");
      }
    }

    // Si la conexión es exitosa, enviamos la hora
    if (client.connected()) {
      client.println(timeStringBuff);
    }
  }

  // Esperamos 1 segundo exacto
  delay(1000);
}