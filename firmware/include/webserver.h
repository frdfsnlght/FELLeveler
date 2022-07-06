#ifndef XWEBSERVER_H
#define XWEBSERVER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WebServer {

    public:

    static WebServer* getInstance();

    AsyncWebServer server = AsyncWebServer(80);
    AsyncEventSource events = AsyncEventSource("/events");

    void setup();

    private:

    static WebServer* instance;
    static DynamicJsonDocument json;
    static String jsonString;

    static void sendAccelerometer(int);
    static void sendLeveler(int);

    WebServer() {}


};

#endif