#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "vector3.h"
#include "secrets.h"

class Config {

    public:

    enum MainMode {
        Tractor,
        Implement
    };

    MainMode mode;
    char name[32];
    char wifiSSID[32];
    char wifiPassword[32];
    bool calibrated;
    Vector3 downLevel;
    Vector3 downTipped;

    static Config* getInstance();

    bool read();
    bool write();

    private:

    static const int ConfigSize = 512;

    static Config* instance;

    Config() {
        mode = Tractor;
        strcpy(name, "Unknown");
        strcpy(wifiSSID, DEFAULT_WIFI_SSID);
        strcpy(wifiPassword, DEFAULT_WIFI_PASSWORD);
        calibrated = false;
        downLevel.set(0, 0, 0);
        downTipped.set(0, 0, 0);
    }

};

#endif
