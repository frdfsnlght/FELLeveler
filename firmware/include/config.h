#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "vector3.h"
#include "callback_list.h"
#include "BTDevice.h"

class Config {

    public:

    static Config* getInstance();
    static const int MaxPairedDevices = 10;
    static const int MaxNameLength = 32;
    static const int MaxWifiSSIDLength = 32;
    static const int MaxWifiPasswordLength = 32;

    enum MainMode {
        Tractor,
        Implement
    };

    struct PairedDevice {
        bool used;
        BTDevice device;
    };

    bool dirty;

    MainMode mode;
    char name[MaxNameLength];
    char wifiSSID[MaxWifiSSIDLength];
    char wifiPassword[MaxWifiPasswordLength];
    bool calibrated;
    Vector3 downLevel;
    Vector3 downTipped;
    Vector3 rollPlane;
    Vector3 pitchPlane;
    PairedDevice pairedDevices[MaxPairedDevices];

    CallbackList dirtyChangedListeners = CallbackList();
    CallbackList settingsChangedListeners = CallbackList();
    CallbackList calibratedChangedListeners = CallbackList();
    CallbackList downLevelChangedListeners = CallbackList();
    CallbackList downTippedChangedListeners = CallbackList();
    CallbackList rollPlaneChangedListeners = CallbackList();
    CallbackList pitchPlaneChangedListeners = CallbackList();
    CallbackList pairedDevicesChangedListeners = CallbackList();

    bool read();
    bool write();

    void setDirty(bool d);
    void setSettings(const char* modeStr, const char* newName, const char* ssid, const char* password);
    void setCalibrated(bool cal);
    void setDownLevel(Vector3 &v);
    void setDownTipped(Vector3 &v);
    void setRollPlane(Vector3 &v);
    void setPitchPlane(Vector3 &v);
    bool hasEmptyPairedDevice();
    bool addPairedDevice(const char* name, const char* address);
    bool removePairedDevice(const char* address);

    private:

    static const int ConfigSize = 2048;

    static Config* instance;

    Config();

};

#endif
