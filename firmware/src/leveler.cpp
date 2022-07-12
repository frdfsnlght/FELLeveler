#include "leveler.h"

#include "accelerometer.h"
#include "config.h"

Leveler* Leveler::instance = nullptr;

Leveler* Leveler::getInstance() {
    if (instance == nullptr) instance = new Leveler();
    return instance;
}

void Leveler::setup() {
    Accelerometer::getInstance()->listeners.add([](void) {
        instance->update();
    });
    Serial.println("Leveler setup complete");
}

void Leveler::calibrateLevel() {
    Vector3 down = Accelerometer::getInstance()->filtered;
    Config::getInstance()->setDownLevel(down);
    Config::getInstance()->setCalibrated(false);
}

void Leveler::calibrateTipped() {
    Vector3 down = Accelerometer::getInstance()->filtered;
    Config::getInstance()->setDownTipped(down);
    Config::getInstance()->setCalibrated(true);
}

void Leveler::update() {
    // TODO: math
    // TODO: call listeners
}

