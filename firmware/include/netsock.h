#ifndef NETSOCK_H
#define NETSOCK_H

#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_bt.h>
#include "callback_list.h"
#include "led.h"
#include "config.h"
#include "BTDevice.h"

class Netsock {

    public:

    static Netsock* getInstance();
    static const int Port = 1099;
    static const char* StateStrings[];

    enum State {
        Idle,
        Connected,
        WaitingForClient,
        ConnectToServer,
        WaitingToConnect
    };

    struct Measurements {
        int roll;
        int pitch;
    };

    CallbackList stateChangedListeners = CallbackList();
    CallbackList remoteDeviceChangedListeners = CallbackList();
    CallbackList measurementsChangedListeners = CallbackList();

    State state;
    char remoteName[Config::MaxNameLength];
    IPAddress remoteAddress;
    Measurements measurements;

    void setup();
    void loop();

    private:

    static const int MaxSendBuffer = 256;
    static const int MaxReceiveBuffer = 256;
    static const int ConnectionAttemptInterval = 5000;

    static Netsock* instance;

    LED led = LED(2, false);
    WiFiServer server;
    WiFiClient client;

    Config::Mode mode;

    int connectionAttempts;
    unsigned long lastConnectionAttemptTime;

    char sendBuffer[MaxSendBuffer];
    int sendBufferStart;
    int sendBufferEnd;
    char receiveBuffer[MaxReceiveBuffer];
    int receiveBufferPos;

    Netsock() {}

    void setState(State newState);
    void setupNetsock();
    void setupServer();
    void setupClient();
    void doConnect();
    void doDisconnect(bool setNextState);
    void resetReceiveBuffer();
    void resetSendBuffer();
    void processReceiveBuffer();
    void setRemoteDevice(const JsonDocument& doc);
    void setMeasurements(const JsonDocument& doc);
    void sendString(String str);
    bool sendChar(const char ch);

    void sendDeviceInfo();
    void sendMeasurements();

    void handleSettingsChanged();
    void handleNetworkStateChanged();

};

#endif
