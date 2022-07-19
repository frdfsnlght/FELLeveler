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

    static const int KeepAliveInterval = 1000;
    static const int MaxRequestBufferLen = 256;
    static const int MaxRequests = 10;

    static WebServer* instance;

    struct WebSocketRequest {
        AsyncWebSocketClient* client;
        char buffer[MaxRequestBufferLen];
    };

    AsyncWebServer server = AsyncWebServer(80);
    AsyncWebSocket ws = AsyncWebSocket("/ws");

    unsigned long lastKeepAliveTime = 0;
    WebSocketRequest webSocketRequests[MaxRequests];
    int webSocketRequestsStart;
    int webSocketRequestsEnd;

    WebServer() {}

    bool enqueueWebSocketRequest(AsyncWebSocketClient* client, const char* msg);
    WebSocketRequest* dequeueWebSocketRequest();

    void processRequest(WebSocketRequest* request);
    bool canSend();
    void sendJSON(JsonDocument &doc, AsyncWebSocketClient* client = nullptr, bool debug = true);
    void sendJSONString(AsyncWebSocketClient* client, int id, const char* msg);
    void sendJSONBoolean(AsyncWebSocketClient* client, int id, bool b);

    // API
    void apiConfigure(AsyncWebSocketClient* client, int id, JsonVariant v);
    void apiCalibrateLevel(AsyncWebSocketClient* client, int id);
    void apiCalibrateTipped(AsyncWebSocketClient* client, int id);
    void apiSaveConfig(AsyncWebSocketClient* client, int id);
    void apiReboot(AsyncWebSocketClient* client, int id);
    void apiTest(AsyncWebSocketClient* client, int id);

    void emitKeepAlive();
    void emitConfigDirty(AsyncWebSocketClient *c);
    void emitConfigSettings(AsyncWebSocketClient *c);
    void emitConfigCalibrated(AsyncWebSocketClient *c);
    void emitWifiRSSI(AsyncWebSocketClient *c);
    void emitNetsockConnected(AsyncWebSocketClient *c);
    void emitNetsockRemoteDevice(AsyncWebSocketClient *c);
    void emitLevelerRoll(AsyncWebSocketClient *c);
    void emitLevelerPitch(AsyncWebSocketClient *c);
    void emitLevelerImplementRoll(AsyncWebSocketClient *c);
    void emitLevelerImplementPitch(AsyncWebSocketClient *c);

    void handleNetworkStateChanged();

};

#endif