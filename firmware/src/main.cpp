#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "sensor.h"
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

    setupSensor();
    setupNetwork();
    setupOTA();
    setupWebserver();

    // TODO: setup bluetooth
    // TODO: setup touch input
    // TODO: setup display

    Serial.println("Ready");
}

void loop() {
    loopSensor();
    loopNetwork();
    loopOTA();
    loopWebserver();

}
