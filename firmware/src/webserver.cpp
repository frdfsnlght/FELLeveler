#include "webserver.h"

#include <SPIFFS.h>

#include "accelerometer.h"
#include "leveler.h"
#include "debug.h"

WebServer* WebServer::instance = nullptr;
DynamicJsonDocument WebServer::json = DynamicJsonDocument(1024);
String WebServer::jsonString = String();

WebServer* WebServer::getInstance() {
    if (instance == nullptr) instance = new WebServer();
    return instance;
}

void WebServer::setup() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
        Serial.println("HTTP GET /");
#endif
        request->send(SPIFFS, "/index.html", String(), false);
    });
    server.serveStatic("/", SPIFFS, "/");

    server.on("/api/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
        Serial.println("HTTP reboot");
#endif
        request->send(200, "text/plain", "OK");
        ESP.restart();
    });

/*
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
        Serial.println("reset pressed");
#endif
        request->send(200, "text/plain", "OK");
    });

    server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
        Serial.println("resetX pressed");
#endif
        request->send(200, "text/plain", "OK");
    });

    server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
        Serial.println("resetY pressed");
#endif
        request->send(200, "text/plain", "OK");
    });

    server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER
        Serial.println("resetZ pressed");
#endif
        request->send(200, "text/plain", "OK");
    });
*/

    // Handle web server events
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.print("Client connected with last message ID ");
            Serial.println(client->lastId());
        }
        // send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", NULL, millis(), 10000);
    });
    
    server.addHandler(&events);


    Accelerometer::getInstance()->listeners.add(sendAccelerometer);
    Leveler::getInstance()->listeners.add(sendLeveler);



    server.begin();
    Serial.println("Webserver setup complete");
}

void WebServer::sendAccelerometer(int ignore) {
    static unsigned long lastSend = 0;
    if ((millis() - lastSend) < 1000) return;
    lastSend = millis();
    json.clear();
    jsonString = "";
    Accelerometer* a = Accelerometer::getInstance();
    json["accX"] = String(a->filtered.x);
    json["accY"] = String(a->filtered.y);
    json["accZ"] = String(a->filtered.z);
    serializeJson(json, jsonString);
    WebServer::getInstance()->events.send(jsonString.c_str(), "accelerometer_readings", millis());
}

void WebServer::sendLeveler(int ignore) {
    static unsigned long lastSend = 0;
    if ((millis() - lastSend) < 1000) return;
    lastSend = millis();
    json.clear();
    jsonString = "";
    //Leveler* l = Leveler::getInstance();
    serializeJson(json, jsonString);
    //WebServer::getInstance()->events.send(jsonString.c_str(), "leveler_readings", millis());
}
