#include <Arduino.h>
#include <SPIFFS.h>

#include "config.h"
#include "display.h"
#include "ui.h"
#include "network.h"
#include "webserver.h"
#include "netsock.h"
#include "accelerometer.h"
#include "leveler.h"
#include "boot_screen.h"

#define FACTORY_RESET_PIN 12

void setup() {
    Serial.begin(115200);
    Serial.println("Booting");

/*
    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        Serial.println("Factory reset detected");
        esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);
        if (pi != NULL) {
            const esp_partition_t* factory = esp_partition_get(pi);
            esp_partition_iterator_release(pi);
            if (esp_ota_boot_partition(factory) == ESP_OK) esp_restart();
        } else {
            Serial.println("Factory partition not found!");
        }
    }
*/

    if (! SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
    }

    if (! Config::getInstance()->read()) {
        Serial.println("No configuration found, using defaults");
    }

    Display::getInstance()->setup();
    Network::getInstance()->setup();
    Accelerometer::getInstance()->setup();
    Leveler::getInstance()->setup();

    UI::getInstance()->setup(BootScreen::getInstance());
    Netsock::getInstance()->setup();
    WebServer::getInstance()->setup();

    Serial.println("Ready ======================");
}

void loop() {
    Network::getInstance()->loop();
    Accelerometer::getInstance()->loop();
    UI::getInstance()->loop();
    Netsock::getInstance()->loop();
    WebServer::getInstance()->loop();
}
