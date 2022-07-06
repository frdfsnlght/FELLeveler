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
        Connected
    };

    CallbackList<int> listeners = CallbackList<int>();
    NetworkState state;
    IPAddress ipAddress;
    int32_t rssi;

    void setup();
    void loop();

    private:

    static Network* instance;

    static const char* Hostname;
    static const char* APSSID;
    static const char* APPassword;
    static const int OTAPort;
    //static const char* OTAHostname;

    int otaUpdateType;

    Network() {}

};


#endif