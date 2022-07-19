#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <DNSServer.h>
#include "callback_list.h"
#include "config.h"

class Network {

    public:

    static Network* getInstance();
    static const char* StateStrings[];

    enum State {
        Idle,
        OTA,
        AP,
        Disconnect,
        Connect,
        Connecting,
        Waiting,
        Connected
    };

    CallbackList stateChangedListeners = CallbackList();
    CallbackList wifiRSSIChangedListeners = CallbackList();

    Config::Mode mode;
    Config::WifiMode wifiMode;
    State state;
    char ssid[Config::MaxSSIDLength];
    char password[Config::MaxPasswordLength];

    IPAddress ipAddress;    // station only
    int32_t rssi;           // station only

    void setup();
    void loop();
    bool hasNetwork();
    
    private:

    static Network* instance;

    static const char* Hostname;
    static const IPAddress APNetmask;

    static const int OTAPort;
    static const int MaxConnectionAttempts;
    static const int ConnectionAttemptInterval;

    DNSServer dnsServer;
    int connectionAttempts;
    unsigned long lastConnectionAttemptTime;
    int otaUpdateType;

    Network() {}

    void setState(State newState);
    void setupWifi();
    void setupAP(const char* apSSID, const char* apPassword);
    void setupStation(const char* stationSSID, const char* stationPassword);

    void handleSettingsChanged();
};


#endif