#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
#include <ArduinoJson.h>
#include <esp_bt.h>
#include "callback_list.h"
#include "led.h"
#include "config.h"
#include "BTDevice.h"

class Bluetooth {

    public:

    static Bluetooth* getInstance();
    static const int MaxScannedDevices = 10;

    enum State {
        Idle,
        Connected,
        WaitingForMaster,
        ScanForSlave,
        ScanningForSlave,
        ConnectToSlave,
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

    CallbackList stateChangedListeners = CallbackList();
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

    static const int MaxSendBuffer = 256;
    static const int MaxReceiveBuffer = 256;
    static const int MaxDeviceName = 64;
    static const int MaxScanTime = 10000;
    static const char BTBaseName[];

    static Bluetooth* instance;


    LED led = LED(2, false);
    BluetoothSerial bt = BluetoothSerial();
    Config::MainMode mode;
    char deviceName[MaxDeviceName];
    bool justConnected;
    bool justDisconnected;
    BTAddress slaveAddress;
    unsigned long startScanTime;

    char sendBuffer[MaxSendBuffer];
    int sendBufferStart;
    int sendBufferEnd;
    char receiveBuffer[MaxReceiveBuffer];
    int receiveBufferPos;

    Bluetooth() {}
    const char* powerLevelToString(esp_power_level_t pl);
    const char* stateToString(State s);
    void setState(State newState);
    void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    void doConnect();
    void doDisconnect();
    void scanForSlave();
    void checkScanTime();
    void checkFoundDevice(BTAdvertisedDevice* device);
    void connectToSlave();
    void completeDeviceScan();

    void resetReceiveBuffer();
    void processReceiveBuffer();
    void setConnectedDevice(const JsonDocument& doc);
    void setMeasurements(const JsonDocument& doc);
    void sendString(String str);
    bool sendChar(const char ch);
    void sendDeviceInfo();
    void sendMeasurements();

};

#endif
