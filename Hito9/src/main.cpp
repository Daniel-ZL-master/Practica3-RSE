#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <ESP32_FTPClient.h>
#include <time.h>

#define WIFI_SSID     "Redmi Note 14 5G"
#define WIFI_PASS     "daniTestAp"

char ftp_server[] = "10.68.114.211";   // IP de tu servidor FileZilla
char ftp_user[]   = "ESP32";
char ftp_pass[]   = "ESP32dani";
const char* FTP_REMOTE_DIR = "/ESP32"; // Carpeta destino en el servidor

// Último argumento: 0 = sin log, 1 = solo errores, 2 = verbose completo
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass, 5000, 2);

const char* NTP_SERVER   = "pool.ntp.org";
const long  GMT_OFFSET_S = 3600;   // UTC+1 (España peninsular). Ajusta si es necesario
const int   DST_OFFSET_S = 3600;   // Horario de verano. Pon 0 en invierno

JsonDocument doc;
JsonArray timeStamps = doc["t"].to<JsonArray>();
JsonArray values     = doc["v"].to<JsonArray>();

unsigned long previousMillis     = 0;
const unsigned long interval     = 1000;   // Muestreo cada 1 s

unsigned long previousMillisSend = 0;
const unsigned long intervalSend = 10000;  // Envío FTP cada 10 s

// DD = día (2 dígitos), MM = mes (2 dígitos), SS = segundos del día (5 dígitos)
String getFilename() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Fallback si NTP aún no está sincronizado
    char buf[32];
    snprintf(buf, sizeof(buf), "grupo01_0000%05lu.json", (millis() / 1000) % 86400);
    return String(buf);
  }
  unsigned long secondsOfDay = timeinfo.tm_hour * 3600UL
                             + timeinfo.tm_min  * 60UL
                             + timeinfo.tm_sec;
  char buf[32];
  snprintf(buf, sizeof(buf), "grupo01_%02d%02d%05lu.json",
           timeinfo.tm_mday,    // DD
           timeinfo.tm_mon + 1, // MM (tm_mon es 0-11)
           secondsOfDay);       // SS (00000-86399)
  return String(buf);
}

void setup() {
  Serial.begin(115200);

  // Campos fijos del JSON
  doc["n"] = "temperatureSensor";
  doc["u"] = "celsius";

  // Conectar WiFi
  Serial.print("Conectando a WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());

  // Sincronizar hora via NTP
  configTime(GMT_OFFSET_S, DST_OFFSET_S, NTP_SERVER);
  Serial.print("Sincronizando NTP");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nHora sincronizada.");
}

void loop() {
  unsigned long now = millis();

  // --- Muestreo cada 1 segundo ---
  if (now - previousMillis >= interval) {
    previousMillis = now;
    timeStamps.add(now);
    values.add(23.4);   // Sustituye por lectura real del sensor
  }

  // --- Envío FTP cada 10 segundos ---
  if (now - previousMillisSend >= intervalSend) {
    previousMillisSend = now;

    // Serializar JSON
    String jsonStr;
    serializeJson(doc, jsonStr);
    Serial.println("Enviando JSON: " + jsonStr);

    // Nombre de archivo
    String filename = getFilename();
    Serial.println("Nombre de archivo: " + filename);

    ftp.OpenConnection();

    if (ftp.isConnected()) {
      ftp.InitFile("Type A"); // Modo ASCII para JSON (texto)
      ftp.NewFile(filename.c_str());
      ftp.Write(jsonStr.c_str());
      ftp.CloseFile();

      Serial.println("Archivo enviado: " + filename);
    } else {
      Serial.println("ERROR: No se pudo conectar al servidor FTP.");
    }

    ftp.CloseConnection();

    // Limpiar arrays para el siguiente ciclo
    timeStamps.clear();
    values.clear();
  }
}