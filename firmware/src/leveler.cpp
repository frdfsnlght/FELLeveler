#include "leveler.h"

#include "accelerometer.h"
#include "config.h"
#include "vector3.h"

Leveler* Leveler::instance = nullptr;

Leveler* Leveler::getInstance() {
    if (instance == nullptr) instance = new Leveler();
    return instance;
}

void Leveler::setup() {
    Accelerometer::getInstance()->listeners.add([](void) {
        instance->update();
    });
    /* TODO
    Netsock::getInstance()->measurementsChangedListeners.add([](void) {
        instance->updateImplement();
    });
    */
    Serial.println("Leveler setup complete");
}

void Leveler::calibrateLevel() {
    Vector3 down = Accelerometer::getInstance()->filtered;
    Config::getInstance()->setDownLevel(down);
    Config::getInstance()->setCalibrated(false);
}

void Leveler::calibrateTipped() {
    Config* config = Config::getInstance();

    Vector3 down = Accelerometer::getInstance()->filtered;
    Vector3 rollPlane, pitchPlane;
    Vector3::cross(down, config->downLevel, pitchPlane);
    pitchPlane.normalize();
    Vector3::cross(config->downLevel, pitchPlane, rollPlane);
    rollPlane.normalize();

    config->setDownTipped(down);
    config->setRollPlane(rollPlane);
    config->setPitchPlane(pitchPlane);
    config->setCalibrated(true);
}

void Leveler::update() {
    static Config* config = Config::getInstance();

    Vector3 down = Accelerometer::getInstance()->filtered;
    down.normalize();
    Vector3 rollProjection, pitchProjection;
    Vector3::projectOnPlane(down, config->rollPlane, rollProjection);
    rollProjection.normalize();
    Vector3::projectOnPlane(down, config->pitchPlane, pitchProjection);
    pitchProjection.normalize();

    int newRoll = Vector3::signedAngle(config->downLevel, rollProjection, config->rollPlane) * 10.0;
    int newPitch = Vector3::signedAngle(config->downLevel, pitchProjection, config->pitchPlane) * 10.0;

    bool changed = false;

    if (newRoll != roll) {
        changed = true;
        roll = newRoll;
        rollChangedListeners.call();
    }
    if (newPitch != pitch) {
        changed = true;
        pitch = newPitch;
        pitchChangedListeners.call();
    }

//    if (changed && (config->mode == Config::Tractor))
//        updateLevel();
}

void Leveler::updateImplement() {
    /* TODO
    Netsock* netsock = Netsock::getInstance();
    bool changed = false;
    if (netsock->measurements.roll != implementRoll) {
        changed = true;
        implementRoll = netsock->measurements.roll;
        implementRollChangedListeners.call();
    }
    if (netsock->measurements.pitch != implementPitch) {
        changed = true;
        implementPitch = netsock->measurements.pitch;
        implementPitchChangedListeners.call();
    }
    if (changed)
        updateLevel();
    */
}

/*
void Leveler::updateLevel() {
    bool newRollFrameLevel = abs(roll - implementRoll) < 10;
    bool newRollEarthLevel = abs(implementRoll) < 10;
    bool newPitchFrameLevel = abs(pitch - implementPitch) < 10;
    bool newPitchEarthLevel = abs(implementPitch) < 10;

    bool changed = false;

    if (newRollFrameLevel != implementRollFrameLevel) {
        changed = true;
        implementRollFrameLevel = newRollFrameLevel;
    }
    if (newRollEarthLevel != implementRollEarthLevel) {
        changed = true;
        implementRollEarthLevel = newRollEarthLevel;
    }
    if (newPitchFrameLevel != implementPitchFrameLevel) {
        changed = true;
        implementPitchFrameLevel = newPitchFrameLevel;
    }
    if (newPitchEarthLevel != implementPitchEarthLevel) {
        changed = true;
        implementPitchEarthLevel = newPitchEarthLevel;
    }

    if (changed)
        implementLevelChangedListeners.call();
}
*/
