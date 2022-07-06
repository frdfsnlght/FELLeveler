#include "bluetooth.h"

#include <Arduino.h>

Bluetooth* Bluetooth::instance = nullptr;

Bluetooth* Bluetooth::getInstance() {
    if (instance == nullptr) instance = new Bluetooth();
    return instance;
}

void Bluetooth::setup() {
    // TODO
    Serial.println("Bluetooth setup complete");
}

void Bluetooth::loop() {
    // TODO
}
