#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include "callback_list.h"
#include "led.h"
#include "config.h"
#include "BTDevice.h"

class Bluetooth {

    public:

    static Bluetooth* getInstance();
    static const int MaxScannedDevices = 10;

    enum State {
        Stopped,
        Connected,
        WaitingForMaster,
        ScanForSlave,
        ScanningForSlave,
        ConnectingToSlave,
        ScanningDevices,
        ScanningComplete
    };

    struct ScannedDevice {
        bool used;
        BTDevice device;
    };

    struct Measurements {
        int roll;
        int pitch;
    };

    CallbackList connectedChangedListeners = CallbackList();
    CallbackList connectedDeviceChangedListeners = CallbackList();
    CallbackList scannedDevicesChangedListeners = CallbackList();
    CallbackList measurementsChangedListeners = CallbackList();

    State state;
    BTDevice connectedDevice;
    ScannedDevice scannedDevices[MaxScannedDevices];
    Measurements measurements;

    void setup();
    void loop();

    void scanDevices();
    bool pairDevice(const char* address);

    private:

    static Bluetooth* instance;

    static const char BTBaseName[];

    LED led = LED(2, false);
    BluetoothSerial bt = BluetoothSerial();
    Config::MainMode mode;
    char deviceName[64];
    char receiveBuffer[256];
    int receiveBufferPos;
    bool justConnected;
    bool justDisconnected;
    BTAddress slaveAddress;

    Bluetooth() {}
    void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    void start();
    void stop();
    void doConnect();
    void doDisconnect();
    void scanForSlave();
    void checkFoundDevice(BTAdvertisedDevice* device);
    void connectToSlave();
    void completeDeviceScan();

    void resetReceiveBuffer();
    void processReceiveBuffer();
    void setConnectedDevice(const JsonDocument& doc);
    void setMeasurements(const JsonDocument& doc);
    void sendString(String str);
    void sendDeviceInfo();
    void sendMeasurements();

};

#endif
