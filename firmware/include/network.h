#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "callback_list.h"

class Network {

    public:

    static Network* getInstance();

    enum NetworkState {
        AP,
        Unconnected,
        Connecting,
        Waiting,
        Connected
    };

    CallbackList<int> listeners = CallbackList<int>();
    NetworkState state;
    IPAddress ipAddress;    // station only
    int32_t rssi;           // station only

    void setup();
    void loop();
    bool hasNetwork();
    
    private:

    static Network* instance;

    static const char* Hostname;
    static const char* APSSID;
    static const char* APPassword;
    static const IPAddress APAddress;
    static const IPAddress APNetmask;

    static const int OTAPort;
    //static const char* OTAHostname;
    static const int MaxConnectionAttempts;
    static const int ConnectionAttemptInterval;

    DNSServer dnsServer;
    int connectionAttempts;
    unsigned long lastConnectionAttemptTime;
    int otaUpdateType;

    Network() {}

    void setupAP();
};


#endif