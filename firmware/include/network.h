#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "config.h"
#include "callback_list.h"
#include "sockio.h"
#include "led.h"

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
        Connected,
        Reboot
    };

    CallbackList stateListeners = CallbackList();
    CallbackList wifiRSSIListeners = CallbackList();

    Config::Mode mode;
    Config::WifiMode wifiMode;
    State state;
    char ssid[Config::MaxSSIDLength];
    char password[Config::MaxPasswordLength];

    IPAddress ipAddress;    // station only
    int32_t rssi;           // station only

    void setup();
    void loop();
    bool available();
    
    private:

    static Network* instance;

    static const char* Hostname;
    static const IPAddress APNetmask;

    static const int OTAPort;
    static const int MaxConnectionAttempts;
    static const int ConnectionAttemptInterval;
    static const int RebootDelay;
    static const int ReportRSSIInterval;

    DNSServer dnsServer;
    int connectionAttempts;
    unsigned long lastConnectionAttemptTime;
    int otaUpdateType;
    unsigned long rebootTimer;

    WebServer webServer;
    SockIOServer webSock;
    SockIOServer* implSock;
    SockIOClient* implClient;

    LED led;

    Network():
        webServer(80),
        webSock(81, "/"),
        implSock(NULL),
        implClient(NULL),
        led(2)
        {}

    void handleSettingsChanged();

    void setState(State newState);

    void setupWifi();
    void setupAP(const char* apSSID, const char* apPassword);
    void setupStation(const char* stationSSID, const char* stationPassword);

    void setupDNSServer();
    void setupOTA();

    void setupWebServer();
    void setupWebSockIO();
    void setupImplSockIO();
    void teardownImplSockIO();

    void activateServices();
    void deactivateServices();


    // ====================================
    // SockIO Web API

    void apiWebTest(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiWebConfigure(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiWebCalibrateLevel(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiWebCalibrateTipped(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiWebSaveConfig(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiWebReboot(SockIOServerClient& client, JsonArray& args, JsonArray& ret);

    void emitWeb(SockIOServerClient *c, const String& event, JsonArray& array);
    void emitWebConfigDirty(SockIOServerClient *c);
    void emitWebConfigSettings(SockIOServerClient *c);
    void emitWebConfigCalibrated(SockIOServerClient *c);
    void emitWebWifiRSSI(SockIOServerClient *c);
    void emitWebAngles(SockIOServerClient *c);
    void emitWebRemoteConnected(SockIOServerClient *c);
    void emitWebRemoteInfo(SockIOServerClient *c);
    void emitWebRemoteAngles(SockIOServerClient *c);

    // ====================================
    // SockIO Implement API

    void apiImplRemoteInfo(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiImplRemoteAngles(SockIOServerClient& client, JsonArray& args, JsonArray& ret);

    void emitImpl(SockIOServerClient *c, const String& event, JsonArray& array);
    void emitImplRemoteInfo(SockIOServerClient *c);
    void emitImplRemoteAngles(SockIOServerClient *c);

};


#endif