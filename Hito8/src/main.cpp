#include <Arduino.h>
#include <ArduinoJson.h>

JsonDocument doc;
JsonArray timeStamps = doc["t"].to<JsonArray>();
JsonArray values = doc["v"].to<JsonArray>();
unsigned long previousMillis = 0;
unsigned long interval = 1000;
unsigned long previousMillisPrint = 0;
unsigned long intervalPrint = 10000;

void setup() {
  Serial.begin(115200);
  doc["n"] = "temperatureSensor";
  doc["u"] = "celsius";
}

void loop() {
  // put your main code here, to run repeatedly:
  if (millis() - previousMillis >= interval) {
    previousMillis = millis();
    timeStamps.add(millis());
    values.add(23.4);
  }
  if (millis() - previousMillisPrint >= intervalPrint) {
    previousMillisPrint = millis();
    serializeJson(doc, Serial);
    Serial.println();
    timeStamps.clear();
    values.clear();
  }
}

