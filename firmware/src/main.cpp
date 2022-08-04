#include <Arduino.h>
#include <SPIFFS.h>

#include "config.h"
#include "display.h"
#include "ui.h"
#include "network.h"
#include "accelerometer.h"
#include "leveler.h"

#define FACTORY_RESET_PIN 12

void debugMemory() {
    uint32_t heap = ESP.getHeapSize();
    uint32_t heapFree = ESP.getFreeHeap();
    uint32_t heapMaxAlloc = ESP.getMaxAllocHeap();
    log_d("MEM: %d, %d, %d", heap, heapFree, heapMaxAlloc);
}

void setup() {
    debugMemory();
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

    Config* config = Config::getInstance();

    if (! config->read()) {
        Serial.println("No configuration found, using defaults");
    }

    Network::getInstance()->setup();
    Accelerometer::getInstance()->setup();
    Leveler::getInstance()->setup();

    if (config->running.enableDisplay) {
        Display::getInstance()->setup();
        UI::getInstance()->setup();
        UI::getInstance()->nextScreen();
    }

    Serial.println("Ready ----------------------------");
    debugMemory();
}

void loop() {
    Network::getInstance()->loop();
    Accelerometer::getInstance()->loop();
    if (Config::getInstance()->running.enableDisplay)
        UI::getInstance()->loop();

    static unsigned long lastMemoryReport = 0;
    if ((millis() - lastMemoryReport) > 1000) {
        lastMemoryReport = millis();
        debugMemory();
    }

    yield();
}
