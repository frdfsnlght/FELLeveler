#ifndef XWEBSERVER_H
#define XWEBSERVER_H

#include <ESPAsyncWebServer.h>

class WebServer {

    public:

    static WebServer* getInstance();

    AsyncWebServer server = AsyncWebServer(80);
    AsyncEventSource events = AsyncEventSource("/events");

    void setup();

    private:

    static WebServer* instance;

    WebServer() {}
    
};

#endif