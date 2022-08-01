#include "config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "secrets.h"

Config::Settings::Settings():
    // TODO: reset defaults
    mode(Tractor),
    //wifiMode(HouseWifi),
    wifiMode(TractorWifi),
    name("Unknown"),
    houseSSID(DEFAULT_HOUSE_SSID),
    housePassword(DEFAULT_HOUSE_PASSWORD),
    tractorSSID(DEFAULT_TRACTOR_SSID),
    tractorPassword(DEFAULT_TRACTOR_PASSWORD),
    tractorAddress(10, 0, 0, 1),
    enableDisplay(true)
    {}

Config::Settings::Settings(Settings& src) {
    mode = src.mode;
    wifiMode = src.wifiMode;
    strcpy(name, src.name);
    strcpy(housePassword, src.housePassword);
    strcpy(tractorSSID, src.tractorSSID);
    strcpy(tractorPassword, src.tractorPassword);
    tractorAddress = src.tractorAddress;
    enableDisplay = src.enableDisplay;
}

Config* Config::instance = nullptr;
const char* Config::ModeStrings[] = {"Tractor", "Implement"};
const char* Config::WifiModeStrings[] = {"HouseWifi", "TractorWifi"};

Config* Config::getInstance() {
    if (instance == nullptr) instance = new Config();
    return instance;
}

Config::Config() {
    dirty = false;
    running = save;
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
        save.mode = Tractor;
    else if (strcmp(doc["mode"], ModeStrings[Implement]) == 0)
        save.mode = Implement;
    if (strcmp(doc["wifiMode"], WifiModeStrings[HouseWifi]) == 0)
        save.wifiMode = HouseWifi;
    else if (strcmp(doc["wifiMode"], WifiModeStrings[TractorWifi]) == 0)
        save.wifiMode = TractorWifi;

    strcpy(save.name, doc["name"]);
    strcpy(save.houseSSID, doc["houseSSID"]);
    strcpy(save.housePassword, doc["housePassword"]);
    strcpy(save.tractorSSID, doc["tractorSSID"]);
    strcpy(save.tractorPassword, doc["tractorPassword"]);
    save.tractorAddress.fromString((const char*)doc["tractorAddress"]);
    save.enableDisplay = doc["enableDisplay"];

    running = save;
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

    doc["mode"] = ModeStrings[save.mode];
    doc["wifiMode"] = WifiModeStrings[save.wifiMode];
    doc["name"] = save.name;
    doc["houseSSID"] = save.houseSSID;
    doc["housePassword"] = save.housePassword;
    doc["tractorSSID"] = save.tractorSSID;
    doc["tractorPassword"] = save.tractorPassword;
    doc["tractorAddress"] = save.tractorAddress.toString();
    doc["enableDisplay"] = save.enableDisplay;
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
    dirtyListeners.call();
}

void Config::setSettings(
    const char* modeStr,
    const char* wifiModeStr,
    const char* newName,
    const char* hSSID,
    const char* hPassword,
    const char* tSSID,
    const char* tPassword,
    const char* tAddress,
    bool enableDisplay) {
    if (strcmp(modeStr, ModeStrings[Tractor]) == 0)
        save.mode = Tractor;
    else if (strcmp(modeStr, ModeStrings[Implement]) == 0)
        save.mode = Implement;
    if (strcmp(wifiModeStr, WifiModeStrings[HouseWifi]) == 0)
        save.wifiMode = HouseWifi;
    else if (strcmp(wifiModeStr, WifiModeStrings[TractorWifi]) == 0)
        save.wifiMode = TractorWifi;
    strcpy(save.name, newName);
    strcpy(save.houseSSID, hSSID);
    strcpy(save.housePassword, hPassword);
    strcpy(save.tractorSSID, tSSID);
    strcpy(save.tractorPassword, tPassword);
    save.tractorAddress.fromString(tAddress);
    save.enableDisplay = enableDisplay;
    settingsListeners.call();
    setDirty(true);
}

void Config::setCalibrated(bool cal) {
    if (cal == calibrated) return;
    calibrated = cal;
    calibratedListeners.call();
    setDirty(true);
}

void Config::setDownLevel(Vector3 &v) {
    if (downLevel == v) return;
    downLevel.set(v);
    downLevelListeners.call();
    setDirty(true);
}

void Config::setDownTipped(Vector3 &v) {
    if (downTipped == v) return;
    downTipped.set(v);
    downTippedListeners.call();
    setDirty(true);
}

void Config::setRollPlane(Vector3 &v) {
    if (rollPlane == v) return;
    rollPlane.set(v);
    rollPlaneListeners.call();
    setDirty(true);
}

void Config::setPitchPlane(Vector3 &v) {
    if (pitchPlane == v) return;
    pitchPlane.set(v);
    pitchPlaneListeners.call();
    setDirty(true);
}
