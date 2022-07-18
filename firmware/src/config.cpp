#include "config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "secrets.h"

Config* Config::instance = nullptr;
const char* Config::ModeStrings[] = {"Tractor", "Implement"};
const char* Config::WifiModeStrings[] = {"HouseWifi", "TractorWifi"};

Config* Config::getInstance() {
    if (instance == nullptr) instance = new Config();
    return instance;
}

Config::Config() {
    dirty = false;
    // TODO: reset defaults
//    mode = Tractor;
//    wifiMode = TractorWifi;
    mode = Tractor;
    wifiMode = HouseWifi;
    strcpy(name, "Unknown");
    strcpy(houseSSID, DEFAULT_HOUSE_SSID);
    strcpy(housePassword, DEFAULT_HOUSE_PASSWORD);
    strcpy(tractorSSID, DEFAULT_TRACTOR_SSID);
    strcpy(tractorPassword, DEFAULT_TRACTOR_PASSWORD);
    tractorAddress.fromString("8.8.8.8");
    calibrated = false;
    downLevel.set(0, 0, 0);
    downTipped.set(0, 0, 0);
    rollPlane.set(0, 0, 0);
    pitchPlane.set(0, 0, 0);
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
    if (strcmp(doc["mode"], ModeStrings[Tractor]) == 0)
        mode = Tractor;
    else if (strcmp(doc["mode"], ModeStrings[Implement]) == 0)
        mode = Implement;
    if (strcmp(doc["wifiMode"], WifiModeStrings[HouseWifi]) == 0)
        wifiMode = HouseWifi;
    else if (strcmp(doc["wifiMode"], WifiModeStrings[TractorWifi]) == 0)
        wifiMode = TractorWifi;

    strcpy(name, doc["name"]);
    strcpy(houseSSID, doc["houseSSID"]);
    strcpy(housePassword, doc["housePassword"]);
    strcpy(tractorSSID, doc["tractorSSID"]);
    strcpy(tractorPassword, doc["tractorPassword"]);
    tractorAddress.fromString((const char*)doc["tractorAddress"]);
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

    doc["mode"] = ModeStrings[mode];
    doc["wifiMode"] = WifiModeStrings[wifiMode];
    doc["name"] = name;
    doc["houseSSID"] = houseSSID;
    doc["housePassword"] = housePassword;
    doc["tractorSSID"] = tractorSSID;
    doc["tractorPassword"] = tractorPassword;
    doc["tractorAddress"] = tractorAddress.toString();
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

void Config::setSettings(
    const char* modeStr,
    const char* wifiModeStr,
    const char* newName,
    const char* hSSID,
    const char* hPassword,
    const char* tSSID,
    const char* tPassword,
    const char* tAddress) {
    if (strcmp(modeStr, ModeStrings[Tractor]) == 0)
        mode = Tractor;
    else if (strcmp(modeStr, ModeStrings[Implement]) == 0)
        mode = Implement;
    if (strcmp(wifiModeStr, WifiModeStrings[HouseWifi]) == 0)
        wifiMode = HouseWifi;
    else if (strcmp(wifiModeStr, WifiModeStrings[TractorWifi]) == 0)
        wifiMode = TractorWifi;
    strcpy(name, newName);
    strcpy(houseSSID, hSSID);
    strcpy(housePassword, hPassword);
    strcpy(tractorSSID, tSSID);
    strcpy(tractorPassword, tPassword);
    tractorAddress.fromString(tAddress);
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
