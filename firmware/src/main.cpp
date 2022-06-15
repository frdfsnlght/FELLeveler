#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "network.h"
#include "ota.h"
#include "webserver.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

    if (! SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    if (! readConfig()) {
        createDefaultConfig();
        Serial.println("No configuration found, using defaults");
    }

    setupNetwork();
    setupOTA();
    setupWebserver();

    // TODO: setup bluetooth
    // TODO: setup accelerometer
    // TODO: setup touch input
    // TODO: setup i2c display

    Serial.println("Ready");
}

void loop() {
    loopNetwork();
    loopOTA();
    loopWebserver();

}
