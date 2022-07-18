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

    bool dirty;

    Mode mode;
    WifiMode wifiMode;
    char name[MaxNameLength];
    char houseSSID[MaxSSIDLength];
    char housePassword[MaxPasswordLength];
    char tractorSSID[MaxSSIDLength];
    char tractorPassword[MaxPasswordLength];
    IPAddress tractorAddress;

    bool calibrated;
    Vector3 downLevel;
    Vector3 downTipped;
    Vector3 rollPlane;
    Vector3 pitchPlane;

    CallbackList dirtyChangedListeners = CallbackList();
    CallbackList settingsChangedListeners = CallbackList();
    CallbackList calibratedChangedListeners = CallbackList();
    CallbackList downLevelChangedListeners = CallbackList();
    CallbackList downTippedChangedListeners = CallbackList();
    CallbackList rollPlaneChangedListeners = CallbackList();
    CallbackList pitchPlaneChangedListeners = CallbackList();

    bool read();
    bool write();

    void setDirty(bool d);
    void setSettings(
        const char* modeStr,
        const char* wifiModeStr,
        const char* newName,
        const char* houseSSID,
        const char* housePassword,
        const char* tractorSSID,
        const char* tractorPassword,
        const char* tractorAddress);
    void setCalibrated(bool cal);
    void setDownLevel(Vector3 &v);
    void setDownTipped(Vector3 &v);
    void setRollPlane(Vector3 &v);
    void setPitchPlane(Vector3 &v);

    private:

    static const int ConfigSize = 2048;

    static Config* instance;

    Config();

};

#endif
