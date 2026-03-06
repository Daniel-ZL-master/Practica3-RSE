#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include "time.h"

// Replace with your network credentials
const char* ssid = "Redmi Note 14 5G";
const char* password = "daniTestAp";

// NTP configuration
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;       // UTC+1 (Spain). Change as needed.
const int   daylightOffset_sec = 3600;  // 1 hour for summer time

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Returns current time as a formatted string "HH:MM:SS"
String getTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Error NTP";
  }
  char timeStr[9];
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
  return String(timeStr);
}

// Returns current date as a formatted string "DD/MM/YYYY"
String getDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "Error NTP";
  }
  char dateStr[11];
  strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);
  return String(dateStr);
}

// Replaces placeholders with NTP time/date values
String processor(const String& var) {
  if (var == "TIME") {
    return getTime();
  }
  if (var == "DATE") {
    return getDate();
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected.");
  Serial.println(WiFi.localIP());

  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("NTP configured.");

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing needed here; the page refreshes on demand
}
