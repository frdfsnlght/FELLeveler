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
    // TODO: reset default
//    mode = Tractor;
    mode = Implement;
    strcpy(name, "Unknown");
    strcpy(wifiSSID, DEFAULT_WIFI_SSID);
    strcpy(wifiPassword, DEFAULT_WIFI_PASSWORD);
    calibrated = false;
    downLevel.set(0, 0, 0);
    downTipped.set(0, 0, 0);
    for (int i = 0; i < MaxPairedDevices; i++)
        pairedDevices[i].used = false;
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
    rollPlane.x = doc["rollPlane"]["x"];
    rollPlane.y = doc["rollPlane"]["y"];
    rollPlane.z = doc["rollPlane"]["z"];
    pitchPlane.x = doc["pitchPlane"]["x"];
    pitchPlane.y = doc["pitchPlane"]["y"];
    pitchPlane.z = doc["pitchPlane"]["z"];

    if (mode == Tractor) {
        JsonArray a = doc["pairedDevices"];
        for (int i = 0; i < MaxPairedDevices; i++) {
            if (i >= a.size()) {
                pairedDevices[i].used = false;
                pairedDevices[i].device.name[0] = '\0';
                pairedDevices[i].device.address[0] = '\0';
            } else {
                pairedDevices[i].used = true;
                strcpy(pairedDevices[i].device.name, a[i]["name"]);
                strcpy(pairedDevices[i].device.address, a[i]["address"]);
            }
        }
    }

    serializeJsonPretty(doc, Serial);
    Serial.println();
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
    v = doc.createNestedObject("rollPlane");
    v["x"] = rollPlane.x;
    v["y"] = rollPlane.y;
    v["z"] = rollPlane.z;
    v = doc.createNestedObject("pitchPlane");
    v["x"] = pitchPlane.x;
    v["y"] = pitchPlane.y;
    v["z"] = pitchPlane.z;

    JsonObject d;
    if (mode == Tractor) {
        JsonArray a = doc.createNestedArray("pairedDevices");
        for (int i = 0; i < MaxPairedDevices; i++) {
            if (pairedDevices[i].used) {
                d = a.createNestedObject();
                d["used"] = pairedDevices[i].used;
                d["name"] = pairedDevices[i].device.name;
                d["address"] = pairedDevices[i].device.address;
            }
        }
    }

    if (! serializeJson(doc, file)) {
        Serial.println("Error writing configuration");
        file.close();
        return false;
    }
    file.close();
    serializeJsonPretty(doc, Serial);
    Serial.println();
    Serial.println("Configuration written");
    setDirty(false);
    return true;
}

void Config::setDirty(bool d) {
    if (d == dirty) return;
    dirty = d;
    dirtyChangedListeners.call();
}

void Config::setSettings(const char* modeStr, const char* newName, const char* ssid, const char* password) {
    if (strcmp(modeStr, "Tractor") == 0)
        mode = Tractor;
    else if (strcmp(modeStr, "Implement") == 0)
        mode = Implement;
    strcpy(name, newName);
    strcpy(wifiSSID, ssid);
    strcpy(wifiPassword, password);
    settingsChangedListeners.call();
    setDirty(true);
}

void Config::setCalibrated(bool cal) {
    if (cal == calibrated) return;
    calibrated = cal;
    calibratedChangedListeners.call();
    setDirty(true);
}

void Config::setDownLevel(Vector3 &v) {
    if (downLevel == v) return;
    downLevel.set(v);
    downLevelChangedListeners.call();
    setDirty(true);
}

void Config::setDownTipped(Vector3 &v) {
    if (downTipped == v) return;
    downTipped.set(v);
    downTippedChangedListeners.call();
    setDirty(true);
}

void Config::setRollPlane(Vector3 &v) {
    if (rollPlane == v) return;
    rollPlane.set(v);
    rollPlaneChangedListeners.call();
    setDirty(true);
}

void Config::setPitchPlane(Vector3 &v) {
    if (pitchPlane == v) return;
    pitchPlane.set(v);
    pitchPlaneChangedListeners.call();
    setDirty(true);
}

bool Config::addPairedDevice(const char* name, const char* address) {
    for (int i = 0; i < MaxPairedDevices; i++) {
        if (! pairedDevices[i].used) {
            strcpy(pairedDevices[i].device.name, name);
            strcpy(pairedDevices[i].device.address, address);
            pairedDevices[i].used = true;
            pairedDevicesChangedListeners.call();
            setDirty(true);
            return true;
        }
    }
    return false;
}

bool Config::removePairedDevice(const char* address) {
    for (int i = 0; i < MaxPairedDevices; i++) {
        if (pairedDevices[i].used &&
            (strcmp(pairedDevices[i].device.address, address) == 0)) {
            pairedDevices[i].used = false;
            pairedDevices[i].device.name[0] = '\0';
            pairedDevices[i].device.address[0] = '\0';
            pairedDevicesChangedListeners.call();
            setDirty(true);
            return true;
        }
    }
    return false;
}
