#ifndef XWEBSERVER_H
#define XWEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WebServer {

    public:

    static WebServer* getInstance();

    void setup();

    private:

    static WebServer* instance;

    AsyncWebServer server = AsyncWebServer(80);
    AsyncEventSource events = AsyncEventSource("/events");

    WebServer() {}

    static void emptyHandler(AsyncWebServerRequest *r) {}

    // API
    void apiConfigure(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t);
    void apiCalibrateLevel(AsyncWebServerRequest *r);
    void apiCalibrateTipped(AsyncWebServerRequest *r);
    void apiScanBTDevices(AsyncWebServerRequest *r);
    void apiPairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t);
    void apiUnpairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t);
    void apiUnpairBT(AsyncWebServerRequest *r);
    void apiStartPairing(AsyncWebServerRequest *r);
    void apiStopPairing(AsyncWebServerRequest *r);
    void apiSaveConfig(AsyncWebServerRequest *r);
    void apiReboot(AsyncWebServerRequest *r);

};

#endif