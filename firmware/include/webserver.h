#ifndef XWEBSERVER_H
#define XWEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WebServer {

    public:

    static WebServer* getInstance();

    void setup();
    void loop();

    private:

    static WebServer* instance;

    AsyncWebServer server = AsyncWebServer(80);
    AsyncEventSource events = AsyncEventSource("/events");

    const int KeepAliveInterval = 1000;
    unsigned long lastKeepAliveTime = 0;
    
    WebServer() {}

    static void emptyHandler(AsyncWebServerRequest *r) {}

    // API
    void apiConfigure(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t);
    void apiCalibrateLevel(AsyncWebServerRequest *r);
    void apiCalibrateTipped(AsyncWebServerRequest *r);
    void apiScanBTDevices(AsyncWebServerRequest *r);
    void apiPairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t);
    void apiUnpairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t);
    void apiSaveConfig(AsyncWebServerRequest *r);
    void apiReboot(AsyncWebServerRequest *r);

    bool canSendEvent();
    void sendEvent(String &msg, const char* event, AsyncEventSourceClient *client = nullptr);

    void emitKeepAlive();
    void emitConfigDirty(AsyncEventSourceClient *c);
    void emitConfigSettings(AsyncEventSourceClient *c);
    void emitConfigCalibrated(AsyncEventSourceClient *c);
    void emitConfigPairedDevices();
    void emitWifiMode(AsyncEventSourceClient *c);
    void emitWifiRSSI(AsyncEventSourceClient *c);
    void emitBTConnected(AsyncEventSourceClient *c);
    void emitBTScannedDevices();
    void emitBTConnectedDevice(AsyncEventSourceClient *c);
    void emitLevelerRoll(AsyncEventSourceClient *c);
    void emitLevelerPitch(AsyncEventSourceClient *c);
    void emitLevelerImplementRoll(AsyncEventSourceClient *c);
    void emitLevelerImplementPitch(AsyncEventSourceClient *c);

};

#endif