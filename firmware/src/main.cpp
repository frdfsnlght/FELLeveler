#include <Arduino.h>
#include <SPIFFS.h>

#include "config.h"
#include "display.h"
#include "ui.h"
#include "network.h"
#include "bluetooth.h"
#include "webserver.h"
#include "accelerometer.h"
#include "leveler.h"
#include "boot_screen.h"


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
    Network::getInstance()->setup();
    Bluetooth::getInstance()->setup();
    Accelerometer::getInstance()->setup();
    Leveler::getInstance()->setup();

    UI::getInstance()->setup(BootScreen::getInstance());
    WebServer::getInstance()->setup();

    Serial.println("Ready");
}

void loop() {
    Network::getInstance()->loop();
    Bluetooth::getInstance()->loop();
    Accelerometer::getInstance()->loop();
    UI::getInstance()->loop();
}
