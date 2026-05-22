#include <Arduino.h>
#include <AdafruitIO.h>
#include "config.h"

#define IO_LOOP_DELAY 5000
unsigned long lastUpdate = 0;
AdafruitIO_Feed *counter = io.feed("rse");
int count = 0;

void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());
}


void setup() {

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while (!Serial)
    ;

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  counter->onMessage(handleMessage);

  // wait for a connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  counter->get();
}

void loop() {
  // put your main code here, to run repeatedly:
  io.run();

  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {
    // save count to the 'counter' feed on Adafruit IO
    Serial.print("sending -> ");
    Serial.println(count);
    counter->save(count);

    // increment the count by 1
    count++;

    // after publishing, store the current time
    lastUpdate = millis();
  }
}