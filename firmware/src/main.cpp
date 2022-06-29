#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "config.h"
#include "display.h"
#include "ui.h"
#include "network.h"
#include "webserver.h"
#include "accelerometer.h"

#include "boot_screen.h"


DynamicJsonDocument readings(1024);

void updateWebPage(Accelerometer* a) {
    readings["accX"] = String(a->filtered.x);
    readings["accY"] = String(a->filtered.y);
    readings["accZ"] = String(a->filtered.z);
    String accString;
    serializeJson(readings, accString);
    WebServer::getInstance()->events.send(accString.c_str(), "accelerometer_readings", millis());
}

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

    if (! SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    if (! Config::getInstance()->read()) {
        Serial.println("No configuration found, using defaults");
    }

    Display::getInstance()->setup();
    UI::getInstance()->setup(BootScreen::getInstance());
    Network::getInstance()->setup();
    WebServer::getInstance()->setup();
    Accelerometer::getInstance()->setup();

    // TODO: setup bluetooth
    // TODO: mode specific code
    
    Accelerometer::getInstance()->accelerometerListeners.add(updateWebPage);

    Serial.println("Ready");
}

void loop() {
    UI::getInstance()->loop();
    Network::getInstance()->loop();
    Accelerometer::getInstance()->loop();
}
