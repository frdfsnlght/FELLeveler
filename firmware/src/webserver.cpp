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

/*
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
*/

    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t *data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.println("WebSocket client connected");
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
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("WebSocket client disconnected");
        } else if (type == WS_EVT_DATA) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len) {
                if (info->opcode == WS_TEXT) {
                    data[len] = 0;
                    WebServer::getInstance()->receiveMessage(client, (const char*)data);
                    return;
                } else {
                    Serial.println("WebSocket received and ignored binary message");
                }
            } else {
                Serial.println("WebSocket received and ignored multi-frame message");
            }
        }

    });
    server.addHandler(&ws);

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

    Serial.println("WebServer setup complete");
}

void WebServer::loop() {
    if (! Network::getInstance()->hasNetwork()) return;

    if ((millis() - lastKeepAliveTime) > KeepAliveInterval) {
        lastKeepAliveTime = millis();
        ws.cleanupClients();
        emitKeepAlive();
    }
}

void WebServer::receiveMessage(AsyncWebSocketClient* client, const char* msg) {
#ifdef DEBUG_WEBSERVER
    Serial.printf("RECEIVE: %s\n", msg);
#endif
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, msg);
    if (err) {
        Serial.printf("WebSocket JSON serialization error: %s\n", err.c_str());
        return;
    }
    const char* method = doc["method"];
    if (strcmp(method, "configure") == 0)
        apiConfigure(client, doc["id"], doc["data"]);
    else if (strcmp(method, "calibrateLevel") == 0)
        apiCalibrateLevel(client, doc["id"]);
    else if (strcmp(method, "calibrateTipped") == 0)
        apiCalibrateTipped(client, doc["id"]);
    else if (strcmp(method, "saveConfig") == 0)
        apiSaveConfig(client, doc["id"]);
    else if (strcmp(method, "reboot") == 0)
        apiReboot(client, doc["id"]);
    else if (strcmp(method, "test") == 0)
        apiTest(client, doc["id"]);
    // more API calls...
    else
        sendJSONString(client, doc["id"], "Unknown method");
}

bool WebServer::canSend() {
    return Network::getInstance()->hasNetwork();
}

void WebServer::sendJSON(JsonDocument &doc, AsyncWebSocketClient *client, bool debug) {
    if (! canSend()) return;
#ifdef DEBUG_WEBSERVER
    if (debug) {
        Serial.print("SEND: ");
        serializeJson(doc, Serial);
        Serial.println();
    }
#endif
    String json = "";
    serializeJson(doc, json);
    if (client)
        client->text(json.c_str());
    else
        ws.textAll(json.c_str());
}

void WebServer::sendJSONString(AsyncWebSocketClient* client, int id, const char* msg) {
    StaticJsonDocument<64> doc;
    if (id > 0) doc["id"] = id;
    doc["data"] = msg;
    sendJSON(doc, client, true);
}

void WebServer::sendJSONBoolean(AsyncWebSocketClient* client, int id, bool b) {
    StaticJsonDocument<64> doc;
    if (id > 0) doc["id"] = id;
    doc["data"] = b;
    sendJSON(doc, client, true);
}

void WebServer::apiConfigure(AsyncWebSocketClient* client, int id, JsonVariant v) {
    DEBUG("API configure");
    if (! v.is<JsonObject>()) {
        sendJSONString(client, id, "Expected object");
    }
    JsonObject obj = v.as<JsonObject>();
    Config* config = Config::getInstance();
    config->setSettings(
        obj["mode"],
        obj["wifiMode"],
        obj["name"],
        obj["houseSSID"],
        obj["housePassword"],
        obj["tractorSSID"],
        obj["tractorPassword"],
        obj["tractorAddress"]);
    sendJSONBoolean(client, id, true);
}

void WebServer::apiCalibrateLevel(AsyncWebSocketClient* client, int id) {
    DEBUG("API calibrateLevel");
    Leveler* leveler = Leveler::getInstance();
    leveler->calibrateLevel();
    sendJSONBoolean(client, id, true);
}

void WebServer::apiCalibrateTipped(AsyncWebSocketClient* client, int id) {
    DEBUG("API calibrateTipped");
    Leveler* leveler = Leveler::getInstance();
    leveler->calibrateTipped();
    sendJSONBoolean(client, id, true);
}

void WebServer::apiSaveConfig(AsyncWebSocketClient* client, int id) {
    DEBUG("API saveConfig");
    Config::getInstance()->write();
    sendJSONBoolean(client, id, true);
}

void WebServer::apiReboot(AsyncWebSocketClient* client, int id) {
    DEBUG("API reboot");
    sendJSONBoolean(client, id, true);
    delay(1000);
    ESP.restart();
}

void WebServer::apiTest(AsyncWebSocketClient* client, int id) {
    DEBUG("API test");
    sendJSONBoolean(client, id, true);
}



void WebServer::emitKeepAlive() {
    StaticJsonDocument<64> doc;
    doc["event"] = "keepAlive";
    doc["data"] = true;
    sendJSON(doc, NULL, false);
}

void WebServer::emitConfigDirty(AsyncWebSocketClient *c) {
    StaticJsonDocument<64> doc;
    doc["event"] = "configDirty";
    doc["data"] = Config::getInstance()->dirty;
    sendJSON(doc, c);
}

void WebServer::emitConfigSettings(AsyncWebSocketClient *c) {
    StaticJsonDocument<256> doc;
    Config* config = Config::getInstance();
    doc["event"] = "settings";
    doc["data"] = doc.createNestedObject();
    doc["data"]["mode"] = Config::ModeStrings[config->mode];
    doc["data"]["wifiMode"] = Config::WifiModeStrings[config->wifiMode];
    doc["data"]["name"] = config->name;
    doc["data"]["houseSSID"] = config->houseSSID;
    doc["data"]["housePassword"] = config->housePassword;
    doc["data"]["tractorSSID"] = config->tractorSSID;
    doc["data"]["tractorPassword"] = config->tractorPassword;
    doc["data"]["tractorAddress"] = config->tractorAddress.toString();
    sendJSON(doc, c);
}

void WebServer::emitConfigCalibrated(AsyncWebSocketClient *c) {
    StaticJsonDocument<64> doc;
    doc["event"] = "calibrated";
    doc["data"] = Config::getInstance()->calibrated;
    sendJSON(doc, c);
}

void WebServer::emitWifiRSSI(AsyncWebSocketClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    doc["event"] = "wifiRSSI";
    doc["data"] = Network::getInstance()->rssi;
    sendJSON(doc, c);
}

void WebServer::emitNetsockConnected(AsyncWebSocketClient *c) {
    StaticJsonDocument<64> doc;
    doc["event"] = "netsockConnected";
    doc["data"] = Netsock::getInstance()->state == Netsock::Connected;
    sendJSON(doc, c);
}

void WebServer::emitNetsockRemoteDevice(AsyncWebSocketClient *c) {
    StaticJsonDocument<192> doc;
    Netsock* netsock = Netsock::getInstance();
    doc["event"] = "netsockRemoteDevice";
    doc["data"] = doc.createNestedObject();
    doc["data"]["name"] = netsock->remoteName;
    doc["data"]["address"] = netsock->remoteAddress.toString();
    sendJSON(doc, c);
}

void WebServer::emitLevelerRoll(AsyncWebSocketClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    doc["event"] = "roll";
    doc["data"] = Leveler::getInstance()->roll;
    sendJSON(doc, c);
}

void WebServer::emitLevelerPitch(AsyncWebSocketClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    doc["event"] = "pitch";
    doc["data"] = Leveler::getInstance()->pitch;
    sendJSON(doc, c);
}

void WebServer::emitLevelerImplementRoll(AsyncWebSocketClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    doc["event"] = "implementRoll";
    doc["data"] = Leveler::getInstance()->implementRoll;
    sendJSON(doc, c);
}

void WebServer::emitLevelerImplementPitch(AsyncWebSocketClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    doc["event"] = "implementPitch";
    doc["data"] = Leveler::getInstance()->implementPitch;
    sendJSON(doc, c);
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
