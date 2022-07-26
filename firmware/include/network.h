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
    SockIOServer sockio;
    LED led;

    Network():
        webServer(80),
        sockio(81, "/ws"),
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
    void setupSockIO();

    void activateServices();
    void deactivateServices();

    // ====================================
    // SockIO API

    void apiTest(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiConfigure(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiCalibrateLevel(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiCalibrateTipped(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiSaveConfig(SockIOServerClient& client, JsonArray& args, JsonArray& ret);
    void apiReboot(SockIOServerClient& client, JsonArray& args, JsonArray& ret);

    void emit(SockIOServerClient *c, const String& event, JsonArray& array);
    void emitConfigDirty(SockIOServerClient *c);
    void emitConfigSettings(SockIOServerClient *c);
    void emitConfigCalibrated(SockIOServerClient *c);
    void emitWifiRSSI(SockIOServerClient *c);
    void emitImplementConnected(SockIOServerClient *c);
    void emitImplementInfo(SockIOServerClient *c);
    void emitRoll(SockIOServerClient *c);
    void emitPitch(SockIOServerClient *c);
    void emitImplementRoll(SockIOServerClient *c);
    void emitImplementPitch(SockIOServerClient *c);


};


#endif