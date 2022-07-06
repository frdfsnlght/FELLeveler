#include "config.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

Config* Config::instance = nullptr;

Config* Config::getInstance() {
    if (instance == nullptr) instance = new Config();
    return instance;
}

bool Config::read() {
    File file = SPIFFS.open("/config.json", FILE_READ);
    if (! file) {
        Serial.println("Unable to read config.json");
        return false;
    }
    StaticJsonDocument<ConfigSize> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Error reading configuraton");
        file.close();
        return false;
    } else {
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
    }
    file.close();
    Serial.println("Configuration read");
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

    if (! serializeJson(doc, file)) {
        Serial.println("Error writing configuration");
        file.close();
        return false;
    }
    file.close();
    Serial.println("Configuration written");
    return true;
}

