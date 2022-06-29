#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPI.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "sensor.h"
#include "network.h"
#include "ota.h"
#include "webserver.h"
#include "ui.h"
#include "display.h"

#include "boot_screen.h"
#include "status_screen.h"
#include "main_screen.h"

BootScreen bootScreen = BootScreen();
StatusScreen statusScreen = StatusScreen();
MainScreen mainScreen = MainScreen();

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

    Display::getInstance()->setup();
    UI::getInstance()->setup(&bootScreen);

    // TODO: setup bluetooth
    // TODO: setup LED
    
    Serial.println("Ready");
}

void loop() {
    loopSensor();
    loopNetwork();
    loopOTA();
    loopWebserver();
    UI::getInstance()->loop();

}
