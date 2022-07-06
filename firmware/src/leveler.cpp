#include "leveler.h"

#include "accelerometer.h"

Leveler* Leveler::instance = nullptr;

Leveler* Leveler::getInstance() {
    if (instance == nullptr) instance = new Leveler();
    return instance;
}

void Leveler::setup() {
    Accelerometer::getInstance()->listeners.add(accelerometerChanged);

    Serial.println("Leveler setup complete");
}

void Leveler::accelerometerChanged(int ignore) {
    // TODO: math
    // TODO: call listeners
    // TODO: send web events? or have a web handler that listens to this?
}
