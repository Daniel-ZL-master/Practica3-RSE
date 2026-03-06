#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>
#include "esp_log.h"  // Librería nativa para logs

// Etiqueta que aparecerá al principio de cada log
static const char *TAG = "NTP_APP"; 

// ---------------------------------------------------------
// Configuración de red WiFi
// ---------------------------------------------------------
const char* ssid       = "Redmi Note 14 5G";
const char* password   = "daniTestAp";

// ---------------------------------------------------------
// Configuración NTP y Zona Horaria
// ---------------------------------------------------------
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "europe.pool.ntp.org";
const char* time_zone  = "CET-1CEST,M3.5.0,M10.5.0/3"; 

// ---------------------------------------------------------
// Callback nativo
// ---------------------------------------------------------
void timeSyncCallback(struct timeval *t) {
  // ESP_LOGI = Nivel "Info"
  ESP_LOGI(TAG, "¡Sincronización de hora disparada por el callback!");
}

void setup() {
  // Aunque usemos ESP_LOG, inicializar el Serial es buena práctica en Arduino
  Serial.begin(115200);
  delay(1000);

  // 1. Conexión a la red WiFi
  ESP_LOGI(TAG, "Iniciando conexión a la red WiFi: %s", ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
  }
  ESP_LOGI(TAG, "Conectado exitosamente. IP asiganada: %s", WiFi.localIP().toString().c_str());

  // 2. Configurar el callback nativo
  sntp_set_time_sync_notification_cb(timeSyncCallback);

  // 3. Inicializar SNTP
  ESP_LOGI(TAG, "Configurando e inicializando cliente NTP...");
  esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, ntpServer1);
  esp_sntp_setservername(1, ntpServer2);
  esp_sntp_init();

  // 4. Configurar la zona horaria
  setenv("TZ", time_zone, 1);
  tzset();

  // 5. Esperar la sincronización
  ESP_LOGI(TAG, "Esperando la hora correcta desde los servidores...");
  int retry = 0;
  const int retry_count = 15;
  
  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
      ESP_LOGI(TAG, "Sincronizando... (Intento %d/%d)", retry, retry_count);
      delay(2000);
  }
  
  if (retry == retry_count) {
      // ESP_LOGE = Nivel "Error" (suele salir en rojo en el monitor serie)
      ESP_LOGE(TAG, "No se pudo sincronizar la hora después de varios intentos.");
  } else {
      ESP_LOGI(TAG, "Reloj del sistema actualizado correctamente.");
  }
}

void loop() {
  struct tm timeinfo;
  
  if (!getLocalTime(&timeinfo)) {
    // ESP_LOGW = Nivel "Warning" (suele salir en amarillo)
    ESP_LOGW(TAG, "Fallo al leer el reloj interno (RTC). La hora no está sincronizada.");
  } else {
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %d de %B de %Y - %H:%M:%S", &timeinfo);
    
    // Imprimimos la hora en nivel Info
    ESP_LOGI(TAG, "Hora local: %s", timeStringBuff);
  }

  delay(5000);
}