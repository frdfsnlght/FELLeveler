#include "webserver.h"

//#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

WebServer* WebServer::instance = nullptr;

WebServer* WebServer::getInstance() {
    if (instance == nullptr) instance = new WebServer();
    return instance;
}

void WebServer::setup() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/index.html", String(), false);
    });
    server.serveStatic("/", SPIFFS, "/");


    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
        // NOP
        request->send(200, "text/plain", "OK");
    });

    server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request){
        // NOP
        request->send(200, "text/plain", "OK");
    });

    server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request){
        // NOP
        request->send(200, "text/plain", "OK");
    });

    server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request){
        // NOP
        request->send(200, "text/plain", "OK");
    });

    // Handle Web Server Events
    events.onConnect([](AsyncEventSourceClient *client){
        if (client->lastId()) {
            Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 10000);
    });
    
    server.addHandler(&events);





    server.begin();
    Serial.println("Webserver setup complete");
}

