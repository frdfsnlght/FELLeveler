#include "webserver.h"

#include <SPIFFS.h>

#include "config.h"
#include "network.h"
#include "netsock.h"
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

    // API calls
    server.on("/api/configure", HTTP_POST, emptyHandler, NULL, [](AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) { instance->apiConfigure(r, d, l, i, t); });
    server.on("/api/calibrateLevel", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiCalibrateLevel(r); });
    server.on("/api/calibrateTipped", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiCalibrateTipped(r); });
    server.on("/api/saveConfig", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiSaveConfig(r); });
    server.on("/api/reboot", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiReboot(r); });

    // Server events
    events.onConnect([](AsyncEventSourceClient *client) {
        Serial.println("HTTP client connected");
        if (client->lastId()) {
            Serial.print("Client connected with last message ID ");
            Serial.println(client->lastId());
        }
        instance->emitConfigDirty(client); delay(10);
        instance->emitConfigSettings(client); delay(10);
        instance->emitConfigCalibrated(client); delay(10);
        instance->emitNetsockConnected(client); delay(10);
        instance->emitNetsockRemoteDevice(client); delay(10);
        instance->emitLevelerPitch(client); delay(10);
        if (Config::getInstance()->mode == Config::Tractor) {
            instance->emitLevelerRoll(client); delay(10);
            instance->emitLevelerImplementRoll(client); delay(10);
            instance->emitLevelerImplementPitch(client); delay(10);
        }
        if (Network::getInstance()->state != Network::AP) {
            instance->emitWifiRSSI(client); delay(10);
        }
    });
    server.addHandler(&events);

    // Catch all
    server.onNotFound([](AsyncWebServerRequest *request) {
#ifdef DEBUG_WEBSERVER_CORS
        if (request->method() == HTTP_OPTIONS)
            request->send(200);
        else
#endif    
        request->send(SPIFFS, "/w/index.html", String(), false);
    });

#ifdef DEBUG_WEBSERVER_CORS
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "content-type");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "PUT,POST,GET,OPTIONS");
#endif    

    Config* config = Config::getInstance();
    Network* network = Network::getInstance();
    Netsock* netsock = Netsock::getInstance();
    Leveler* leveler = Leveler::getInstance();

    // Subscribe to listeners
    config->dirtyChangedListeners.add([](void) {
        WebServer::instance->emitConfigDirty(nullptr);
    });
    config->settingsChangedListeners.add([](void) {
        WebServer::instance->emitConfigSettings(nullptr);
    });
    config->calibratedChangedListeners.add([](void) {
        WebServer::instance->emitConfigCalibrated(nullptr);
    });

    network->stateChangedListeners.add([](void) {
        instance->handleNetworkStateChanged();
    });
    network->wifiRSSIChangedListeners.add([](void) {
        WebServer::instance->emitWifiRSSI(nullptr);
    });

    netsock->stateChangedListeners.add([](void) {
        WebServer::instance->emitNetsockConnected(nullptr);
    });
    netsock->remoteDeviceChangedListeners.add([](void) {
        WebServer::instance->emitNetsockRemoteDevice(nullptr);
    });

    leveler->rollChangedListeners.add([](void) {
        WebServer::instance->emitLevelerRoll(nullptr);
    });
    leveler->implementRollChangedListeners.add([](void) {
        WebServer::instance->emitLevelerImplementRoll(nullptr);
    });
    leveler->implementPitchChangedListeners.add([](void) {
        WebServer::instance->emitLevelerImplementPitch(nullptr);
    });
    leveler->pitchChangedListeners.add([](void) {
        WebServer::instance->emitLevelerPitch(nullptr);
    });

    Serial.println("Webserver setup complete");
}

void WebServer::loop() {
    if (! Network::getInstance()->hasNetwork()) return;

    if ((millis() - lastKeepAliveTime) > KeepAliveInterval) {
        lastKeepAliveTime = millis();
        emitKeepAlive();
    }
}

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
    config->setSettings(
        doc["mode"],
        doc["wifiMode"],
        doc["name"],
        doc["houseSSID"],
        doc["housePassword"],
        doc["tractorSSID"],
        doc["tractorPassword"],
        doc["tractorAddress"]);
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

void WebServer::apiSaveConfig(AsyncWebServerRequest *r) {
    DEBUG("HTTP saveConfig");
    Config::getInstance()->write();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiReboot(AsyncWebServerRequest *r) {
    DEBUG("HTTP reboot");
    r->send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
}

bool WebServer::canSendEvent() {
    return Network::getInstance()->hasNetwork();
}

void WebServer::sendEvent(String &msg, const char* event, AsyncEventSourceClient *client) {
    if (! canSendEvent()) return;
#ifdef DEBUG_WEBSERVER
    Serial.print("EVENT: ");
    Serial.print(event);
    Serial.print(" -> ");
    Serial.println(msg.c_str());
#endif
    if (client)
        client->send(msg.c_str(), event, millis());
    else
        events.send(msg.c_str(), event, millis());
}

void WebServer::emitKeepAlive() {
    if (! canSendEvent()) return;
    events.send("keepAlive", "keepAlive", millis());
}

void WebServer::emitConfigDirty(AsyncEventSourceClient *c) {
    String str = Config::getInstance()->dirty ? "true" : "false";
    sendEvent(str, "configDirty", c);
}

void WebServer::emitConfigSettings(AsyncEventSourceClient *c) {
    String json = "";
    StaticJsonDocument<256> doc;
    Config* config = Config::getInstance();
    doc["mode"] = Config::ModeStrings[config->mode];
    doc["wifiMode"] = Config::WifiModeStrings[config->wifiMode];
    doc["name"] = config->name;
    doc["houseSSID"] = config->houseSSID;
    doc["housePassword"] = config->housePassword;
    doc["tractorSSID"] = config->tractorSSID;
    doc["tractorPassword"] = config->tractorPassword;
    doc["tractorAddress"] = config->tractorAddress.toString();
    serializeJson(doc, json);
    sendEvent(json, "settings", c);
}

void WebServer::emitConfigCalibrated(AsyncEventSourceClient *c) {
    String str = Config::getInstance()->calibrated ? "true" : "false";
    sendEvent(str, "calibrated", c);
}

void WebServer::emitWifiRSSI(AsyncEventSourceClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    String str = String(Network::getInstance()->rssi);
    sendEvent(str, "wifiRSSI", c);
}

void WebServer::emitNetsockConnected(AsyncEventSourceClient *c) {
    String str = (Netsock::getInstance()->state == Netsock::Connected) ? "true" : "false";
    sendEvent(str, "netsockConnected", c);
}

void WebServer::emitNetsockRemoteDevice(AsyncEventSourceClient *c) {
    String json = "";
    StaticJsonDocument<128> doc;
    Netsock* netsock = Netsock::getInstance();
    doc["name"] = netsock->remoteName;
    doc["address"] = netsock->remoteAddress.toString();
    serializeJson(doc, json);
    sendEvent(json, "netsockRemoteDevice");
}

void WebServer::emitLevelerRoll(AsyncEventSourceClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    String str = String(Leveler::getInstance()->roll);
    sendEvent(str, "roll", c);
}

void WebServer::emitLevelerPitch(AsyncEventSourceClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    String str = String(Leveler::getInstance()->pitch);
    sendEvent(str, "pitch", c);
}

void WebServer::emitLevelerImplementRoll(AsyncEventSourceClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    String str = String(Leveler::getInstance()->implementRoll);
    sendEvent(str, "implementRoll", c);
}

void WebServer::emitLevelerImplementPitch(AsyncEventSourceClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    String str = String(Leveler::getInstance()->implementPitch);
    sendEvent(str, "implementPitch", c);
}

void WebServer::handleNetworkStateChanged() {
    Network* network = Network::getInstance();
    if (network->state == Network::Connected) {
        Serial.println("Webserver starting");
        server.begin();

    } else if ((network->state == Network::Disconnect) || (network->state == Network::OTA)) {
        Serial.println("Webserver stopping");
        server.end();

    }
}
