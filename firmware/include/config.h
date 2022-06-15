#ifndef CONFIG_H
#define CONFIG_H

const int CONFIG_SIZE = 512;

struct Config {
    char wifiSSID[32];
    char wifiPassword[32];
};

Config config;

bool readConfig() {
    File file = SPIFFS.open("config.json", FILE_READ);
    if (! file) {
        Serial.println("Unable to read config.json");
        return false;
    }
    StaticJsonDocument<CONFIG_SIZE> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Error reading configuraton");
        file.close();
        return false;
    } else {
        strcpy(config.wifiSSID, doc["wifiSSID"]);
        strcpy(config.wifiPassword, doc["wifiPassword"]);
    }
    file.close();
    Serial.println("Configuration read");
    return true;
}

bool writeConfig() {
    File file = SPIFFS.open("config.json", FILE_WRITE);
    if (! file) {
        Serial.println("Unable to write to config.json");
        return false;
    }
    StaticJsonDocument<CONFIG_SIZE> doc;
    doc["wifiSSID"] = config.wifiSSID;
    doc["wifiPassword"] = config.wifiPassword;
    if (! serializeJson(doc, file)) {
        Serial.println("Error writing configuration");
        file.close();
        return false;
    }
    file.close();
    Serial.println("Configuration written");
    return true;
}

void createDefaultConfig() {
    strcpy(config.wifiSSID, "");
    strcpy(config.wifiPassword, "");
}

#endif
