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

    log_d("Down level: %s", config->downLevel.toString().c_str());
    log_d("Down tipped: %s", config->downTipped.toString().c_str());
    log_d("Roll plane: %s", config->rollPlane.toString().c_str());
    log_d("Pitch plane: %s", config->pitchPlane.toString().c_str());
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

    if ((newRoll != roll) || (newPitch != pitch)) {
        roll = newRoll;
        pitch = newPitch;
        anglesListeners.call();
    }
}

void Leveler::setRemoteConnected(bool b) {
    if (remoteConnected == b) return;
    remoteConnected = b;
    if (! remoteConnected) {
        remoteName = "";
        remoteAddress = "";
        remoteRoll = 0;
        remotePitch = 0;
    }
    remoteConnectedListeners.call();
    if (! remoteConnected) {
        remoteInfoListeners.call();
        remoteAnglesListeners.call();
    }
}

void Leveler::setRemoteInfo(const String& name, const String& address) {
    if (remoteName == name && remoteAddress == address) return;
    remoteName = name;
    remoteAddress = address;
    remoteInfoListeners.call();
}

void Leveler::setRemoteAngles(int roll, int pitch) {
    if (remoteRoll == roll && remotePitch == pitch) return;
    remoteRoll = roll;
    remotePitch = pitch;
    remoteAnglesListeners.call();
}

