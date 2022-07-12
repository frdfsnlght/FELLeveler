#include "webserver.h"

#include <SPIFFS.h>

#include "config.h"
#include "bluetooth.h"
//#include "accelerometer.h"
#include "leveler.h"
#include "debug.h"

#ifdef DEBUG_WEBSERVER
    #define DEBUG(msg) Serial.println(msg)
#else
    #define DEBUG(msg)
#endif

WebServer* WebServer::instance = nullptr;

WebServer* WebServer::getInstance() {
    if (instance == nullptr) instance = new WebServer();
    return instance;
}

void WebServer::setup() {

    // Static content
    server.serveStatic("/", SPIFFS, "/w/").setDefaultFile("index.html");
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/w/index.html", String(), false);
    });

    // API calls
    server.on("/api/configure", HTTP_POST, emptyHandler, NULL, [](AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) { instance->apiConfigure(r, d, l, i, t); });
    server.on("/api/calibrateLevel", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiCalibrateLevel(r); });
    server.on("/api/calibrateTipped", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiCalibrateTipped(r); });
    server.on("/api/scanBTDevices", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiScanBTDevices(r); });
    server.on("/api/pairBTDevice", HTTP_POST, emptyHandler, NULL, [](AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) { instance->apiPairBTDevice(r, d, l, i, t); });
    server.on("/api/unpairBTDevice", HTTP_POST, emptyHandler, NULL, [](AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) { instance->apiUnpairBTDevice(r, d, l, i, t); });
    server.on("/api/unpairBT", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiUnpairBT(r); });
    server.on("/api/startPairing", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiStartPairing(r); });
    server.on("/api/stopPairing", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiStopPairing(r); });
    server.on("/api/saveConfig", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiSaveConfig(r); });
    server.on("/api/reboot", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiReboot(r); });

    // Server events
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.print("Client connected with last message ID ");
            Serial.println(client->lastId());
        }
        // TODO: send a bunch of stuff
        client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);

//    Accelerometer::getInstance()->listeners.add(sendAccelerometer);
//    Leveler::getInstance()->listeners.add(sendLeveler);

    server.begin();
    Serial.println("Webserver setup complete");
}

/*
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
*/


void WebServer::apiConfigure(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) {
    DEBUG("HTTP configure");
    DEBUG((const char*)d);
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, d);
    if (err) {
        r->send(400, "text/plain", err.c_str());
        return;
    }
    Config* config = Config::getInstance();
    config->setMode(doc["mode"]);
    config->setName(doc["name"]);
    config->setWifiSSID(doc["wifiSSID"]);
    config->setWifiPassword(doc["wifiPassword"]);
    r->send(200, "text/plain", "OK");
}

void WebServer::apiCalibrateLevel(AsyncWebServerRequest *r) {
    DEBUG("HTTP calibrateLevel");
    Leveler* leveler = Leveler::getInstance();
    leveler->calibrateLevel();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiCalibrateTipped(AsyncWebServerRequest *r) {
    DEBUG("HTTP calibrateTipped");
    Leveler* leveler = Leveler::getInstance();
    leveler->calibrateTipped();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiScanBTDevices(AsyncWebServerRequest *r) {
    DEBUG("HTTP scanBTDevices");
    if (! Config::getInstance()->isTractorMode()) {
        r->send(400, "text/plain", "Tractor only");
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    bt->scanDevices();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiPairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) {
    DEBUG("HTTP pairDevice");
    if (! Config::getInstance()->isTractorMode()) {
        r->send(400, "text/plain", "Tractor only");
        return;
    }
    DEBUG((const char*)d);
    StaticJsonDocument<64> doc;
    DeserializationError err = deserializeJson(doc, d);
    if (err) {
        r->send(400, "text/plain", err.c_str());
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    if (! bt->canPairDevice()) {
        r->send(400, "text/plain", "No room for implement");
        return;
    }
    bt->pairDevice(doc["address"]);
    r->send(200, "text/plain", "OK");
}

void WebServer::apiUnpairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) {
    DEBUG("HTTP unpairDevice");
    if (! Config::getInstance()->isTractorMode()) {
        r->send(400, "text/plain", "Tractor only");
        return;
    }
    DEBUG((const char*)d);
    StaticJsonDocument<64> doc;
    DeserializationError err = deserializeJson(doc, d);
    if (err) {
        r->send(400, "text/plain", err.c_str());
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    if (! bt->canUnpairDevice()) {
        r->send(400, "text/plain", "Device not paired");
        return;
    }
    bt->unpairDevice(doc["address"]);
    r->send(200, "text/plain", "OK");
}

void WebServer::apiUnpairBT(AsyncWebServerRequest *r) {
    DEBUG("HTTP unpairBT");
    if (! Config::getInstance()->isImplementMode()) {
        r->send(400, "text/plain", "Implement only");
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    bt->unpair();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiStartPairing(AsyncWebServerRequest *r) {
    DEBUG("HTTP startPairing");
    if (! Config::getInstance()->isImplementMode()) {
        r->send(400, "text/plain", "Implement only");
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    bt->startPairing();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiStopPairing(AsyncWebServerRequest *r) {
    DEBUG("HTTP stopPairing");
    if (! Config::getInstance()->isImplementMode()) {
        r->send(400, "text/plain", "Implement only");
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    bt->stopPairing();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiSaveConfig(AsyncWebServerRequest *r) {
    DEBUG("HTTP saveConfig");
    Config::getInstance()->save();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiReboot(AsyncWebServerRequest *r) {
    DEBUG("HTTP reboot");
    r->send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
}

