#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
#include "callback_list.h"
#include "led.h"

class Bluetooth {

    public:

    static Bluetooth* getInstance();
    static const int MaxScannedDevices = 10;

    struct BTDevice {
        bool used;
        char name[32];
        char address[18];
    };

    CallbackList connectedChangedListeners = CallbackList();
    CallbackList scannedDevicesChangedListeners = CallbackList();
    CallbackList connectedDeviceChangedListeners = CallbackList();
    CallbackList pairedChangedListeners = CallbackList();

    bool connected;

    // Master
    BTDevice connectedDevice;
    BTDevice scannedDevices[MaxScannedDevices];

    // Slave
    bool paired;

    void setup();
    void loop();
    void scanDevices();
    bool canPairDevice();
    void pairDevice(const char* address);
    bool canUnpairDevice(const char* address);
    void unpairDevice(const char* address);
    void unpair();
    void startPairing();
    void stopPairing();

    private:

    static Bluetooth* instance;

    static const String BTBaseName;

    LED led = LED(2, false);
    BluetoothSerial bt = BluetoothSerial();
    bool master;
    
    Bluetooth() {}
    void handleCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    void receiveData();
    
};

#endif
