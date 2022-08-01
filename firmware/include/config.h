#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include "vector3.h"
#include "callback_list.h"

class Config {

    public:

    static const int MaxNameLength = 32;
    static const int MaxSSIDLength = 32;
    static const int MaxPasswordLength = 32;
    static const char* ModeStrings[];
    static const char* WifiModeStrings[];

    static Config* getInstance();

    enum Mode {
        Tractor,
        Implement
    };

    enum WifiMode {
        HouseWifi,
        TractorWifi
    };

    struct Settings {
        Mode mode;
        WifiMode wifiMode;
        char name[MaxNameLength];
        char houseSSID[MaxSSIDLength];
        char housePassword[MaxPasswordLength];
        char tractorSSID[MaxSSIDLength];
        char tractorPassword[MaxPasswordLength];
        IPAddress tractorAddress;
        bool enableDisplay;
        Settings();
        Settings(Settings& src);
    };

    bool dirty;

    Settings running;
    Settings save;
    bool calibrated;
    Vector3 downLevel;
    Vector3 downTipped;
    Vector3 rollPlane;
    Vector3 pitchPlane;

    CallbackList dirtyListeners = CallbackList();
    CallbackList settingsListeners = CallbackList();
    CallbackList calibratedListeners = CallbackList();
    CallbackList downLevelListeners = CallbackList();
    CallbackList downTippedListeners = CallbackList();
    CallbackList rollPlaneListeners = CallbackList();
    CallbackList pitchPlaneListeners = CallbackList();

    bool read();
    bool write();

    void setSettings(
        const char* modeStr,
        const char* wifiModeStr,
        const char* newName,
        const char* houseSSID,
        const char* housePassword,
        const char* tractorSSID,
        const char* tractorPassword,
        const char* tractorAddress,
        bool enableDisplay);
    void setCalibrated(bool cal);
    void setDownLevel(Vector3 &v);
    void setDownTipped(Vector3 &v);
    void setRollPlane(Vector3 &v);
    void setPitchPlane(Vector3 &v);

    private:

    static const int ConfigSize = 768;

    static Config* instance;

    Config();

    void setDirty(bool d);

};

#endif
