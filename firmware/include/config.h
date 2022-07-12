#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "vector3.h"
#include "callback_list.h"

class Config {

    public:

    static Config* getInstance();
    static const int MaxBTDevices = 10;

    enum MainMode {
        Tractor,
        Implement
    };

    struct BTDevice {
        bool used;
        char name[32];
        char address[18];
    };

    bool dirty;

    MainMode mode;
    char name[32];
    char wifiSSID[32];
    char wifiPassword[32];
    bool calibrated;
    Vector3 downLevel;
    Vector3 downTipped;
    BTDevice pairedDevices[MaxBTDevices];
    BTDevice pairedDevice;

    CallbackList dirtyChangedListeners = CallbackList();
    CallbackList settingsChangedListeners = CallbackList();
    CallbackList calibratedChangedListeners = CallbackList();
    CallbackList downLevelChangedListeners = CallbackList();
    CallbackList downTippedChangedListeners = CallbackList();
    CallbackList pairedDevicesChangedListeners = CallbackList();
    CallbackList pairedDeviceChangedListeners = CallbackList();

    bool read();
    bool write();

    void setDirty(bool d);
    void setSettings(const char* modeStr, const char* newName, const char* ssid, const char* password);
    void setCalibrated(bool cal);
    void setDownLevel(Vector3 &v);
    void setDownTipped(Vector3 &v);
    void setPairedDevice(int i, const char* name, const char* address);
    void setPairedDevice(const char* name, const char* address);

    private:

    static const int ConfigSize = 2048;

    static Config* instance;

    Config();

};

#endif
