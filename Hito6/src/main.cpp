#include <Arduino.h>
#include <WiFi.h>
#include "esp_log.h"

static const char *TAG = "ACCEL APP";
const char* ssid = "Redmi Note 14 5G";
const char* password = "daniTestAp";

const char* server_ip = "10.68.114.211";
const uint16_t server_port = 8080;

WiFiClient client;

bool isSending = false;
unsigned long lastMillis = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  ESP_LOGI(TAG, "Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
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

  if (isSending && millis() - lastMillis > 1000) {
    lastMillis = millis();
    // Enviar datos
    char buffer [100];
    sprintf(buffer, "%d;%d;%d\n", random(-5, 5), random(-5, 5), random(-5, 5));
    client.print(buffer);
    ESP_LOGI(TAG, "Datos enviados: %s", buffer);
  }
  delay(10);
}