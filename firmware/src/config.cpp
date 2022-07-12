#include "config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "secrets.h"

Config* Config::instance = nullptr;

Config* Config::getInstance() {
    if (instance == nullptr) instance = new Config();
    return instance;
}

Config::Config() {
    dirty = false;
    mode = Tractor;
    strcpy(name, "Unknown");
    strcpy(wifiSSID, DEFAULT_WIFI_SSID);
    strcpy(wifiPassword, DEFAULT_WIFI_PASSWORD);
    calibrated = false;
    downLevel.set(0, 0, 0);
    downTipped.set(0, 0, 0);
    for (int i = 0; i < MaxBTDevices; i++)
        pairedDevices[i].used = false;
    pairedDevice.used = false;
}

bool Config::read() {
    File file = SPIFFS.open("/config.json", FILE_READ);
    if (! file) {
        Serial.println("Unable to read config.json");
        return false;
    }
    StaticJsonDocument<ConfigSize> doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();
    if (err) {
        Serial.print("Error reading configuraton: ");
        Serial.println(err.c_str());
        return false;
    }
    if (strcmp(doc["mode"], "Tractor") == 0)
        mode = Tractor;
    else if (strcmp(doc["mode"], "Implement") == 0)
        mode = Implement;
    strcpy(name, doc["name"]);
    strcpy(wifiSSID, doc["wifiSSID"]);
    strcpy(wifiPassword, doc["wifiPassword"]);
    calibrated = doc["calibrated"];
    downLevel.x = doc["downLevel"]["x"];
    downLevel.y = doc["downLevel"]["y"];
    downLevel.z = doc["downLevel"]["z"];
    downTipped.x = doc["downTipped"]["x"];
    downTipped.y = doc["downTipped"]["y"];
    downTipped.z = doc["downTipped"]["z"];

    // TODO: read pairedDevices (tractor)
    // TODO: read pairedDevice (implement)

    Serial.println("Configuration read");
    setDirty(false);
    return true;
}

bool Config::write() {
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    if (! file) {
        Serial.println("Unable to write to config.json");
        return false;
    }
    StaticJsonDocument<ConfigSize> doc;

    if (mode == Tractor)
        doc["mode"] = "Tractor";
    else if (mode == Implement)
        doc["mode"] = "Implement";
    doc["name"] = name;
    doc["wifiSSID"] = wifiSSID;
    doc["wifiPassword"] = wifiPassword;
    doc["calibrated"] = calibrated;
    JsonObject v = doc.createNestedObject("downLevel");
    v["x"] = downLevel.x;
    v["y"] = downLevel.y;
    v["z"] = downLevel.z;
    v = doc.createNestedObject("downTipped");
    v["x"] = downTipped.x;
    v["y"] = downTipped.y;
    v["z"] = downTipped.z;

    // TODO: write pairedDevices (tractor)
    // TODO: write pairedDevice (implement)

    if (! serializeJson(doc, file)) {
        Serial.println("Error writing configuration");
        file.close();
        return false;
    }
    file.close();
    serializeJsonPretty(doc, Serial);
    Serial.println("Configuration written");
    setDirty(false);
    return true;
}

void Config::setDirty(bool d) {
    if (d == dirty) return;
    dirty = d;
    // TODO: trigger listeners
}

void Config::setMode(const char* modeStr) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setName(const char* newName) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setWifiSSID(const char* ssid) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setWifiPassword(const char* password) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setCalibrated(bool cal) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setDownLevel(Vector3 v) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setDownTipped(Vector3 v) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setPairedDevice(int i, const char* name, const char* address) {
    // TODO: trigger listeners
    setDirty(true);
}

void Config::setPairedDevice(const char* name, const char* address) {
    // TODO: trigger listeners
    setDirty(true);
}

