#include "webserver.h"

#include <SPIFFS.h>

#include "config.h"
#include "network.h"
#include "bluetooth.h"
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
    server.on("/api/saveConfig", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiSaveConfig(r); });
    server.on("/api/reboot", HTTP_GET, [](AsyncWebServerRequest *r) { instance->apiReboot(r); });

    Config* config = Config::getInstance();
    Network* network = Network::getInstance();
    Bluetooth* bt = Bluetooth::getInstance();
    Leveler* leveler = Leveler::getInstance();

    // Server events
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.print("Client connected with last message ID ");
            Serial.println(client->lastId());
        }
        instance->emitConfigDirty(client);
        instance->emitConfigSettings(client);
        instance->emitConfigCalibrated(client);
        instance->emitWifiMode(client);
        instance->emitBTConnected(client);
        instance->emitBTConnectedDevice(client);
        instance->emitLevelerPitch(client);
        if (Config::getInstance()->mode == Config::Tractor) {
            instance->emitLevelerRoll(client);
            instance->emitLevelerImplementRoll(client);
            instance->emitLevelerImplementPitch(client);
        }
        if (Network::getInstance()->state != Network::AP) {
            instance->emitWifiRSSI(client);
        }
    });
    server.addHandler(&events);

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
    config->pairedDevicesChangedListeners.add([](void) {
        WebServer::instance->emitConfigPairedDevices();
    });

    network->wifiModeChangedListeners.add([](void) {
        WebServer::instance->emitWifiMode(nullptr);
    });
    network->wifiRSSIChangedListeners.add([](void) {
        WebServer::instance->emitWifiRSSI(nullptr);
    });

    bt->connectedChangedListeners.add([](void) {
        WebServer::instance->emitBTConnected(nullptr);
    });
    bt->scannedDevicesChangedListeners.add([](void) {
        WebServer::instance->emitBTScannedDevices();
    });
    bt->connectedDeviceChangedListeners.add([](void) {
        WebServer::instance->emitBTConnectedDevice(nullptr);
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

    server.begin();
    Serial.println("Webserver setup complete");
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
    config->setSettings(doc["mode"], doc["name"], doc["wifiSSID"], doc["wifiPassword"]);
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
    if (Config::getInstance()->mode != Config::Tractor) {
        r->send(400, "text/plain", "Tractor only");
        return;
    }
    Bluetooth* bt = Bluetooth::getInstance();
    bt->scanDevices();
    r->send(200, "text/plain", "OK");
}

void WebServer::apiPairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) {
    DEBUG("HTTP pairDevice");
    if (Config::getInstance()->mode != Config::Tractor) {
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
    if (! bt->pairDevice(doc["address"])) {
        r->send(400, "text/plain", "Unknown device or out of room");
        return;
    }
    r->send(200, "text/plain", "OK");
}

void WebServer::apiUnpairBTDevice(AsyncWebServerRequest *r, uint8_t *d, size_t l, size_t i, size_t t) {
    DEBUG("HTTP unpairDevice");
    if (Config::getInstance()->mode != Config::Tractor) {
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
    Config* config = Config::getInstance();
    if (! config->removePairedDevice(doc["address"])) {
        r->send(400, "text/plain", "Device not paired");
        return;
    }
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

void WebServer::emitConfigDirty(AsyncEventSourceClient *c) {
    String str = Config::getInstance()->dirty ? "true" : "false";
    sendEvent(str, "configDirty", c);
}

void WebServer::emitConfigSettings(AsyncEventSourceClient *c) {
    String json = "";
    StaticJsonDocument<256> doc;
    Config* config = Config::getInstance();
    if (config->mode == Config::Tractor)
        doc["mode"] = "Tractor";
    else if (config->mode == Config::Implement)
        doc["mode"] = "Implement";
    doc["name"] = config->name;
    doc["wifiSSID"] = config->wifiSSID;
    doc["wifiPassword"] = config->wifiPassword;
    serializeJson(doc, json);
    sendEvent(json, "settings", c);
}

void WebServer::emitConfigCalibrated(AsyncEventSourceClient *c) {
    String str = Config::getInstance()->calibrated ? "true" : "false";
    sendEvent(str, "calibrated", c);
}

void WebServer::emitConfigPairedDevices() {
    String json = "";
    StaticJsonDocument<1536> doc;
    Config* config = Config::getInstance();
    for (int i = 0; i < Config::MaxPairedDevices; i++) {
        if (! config->pairedDevices[i].used) continue;
        JsonObject d = doc.createNestedObject();
        d["name"] = config->pairedDevices[i].device.name;
        d["address"] = config->pairedDevices[i].device.address;
    }
    serializeJson(doc, json);
    sendEvent(json, "btPairedDevices");
}

void WebServer::emitWifiMode(AsyncEventSourceClient *c) {
    String str = (Network::getInstance()->state == Network::AP) ? "AP" : "Station";
    sendEvent(str, "wifiMode", c);
}

void WebServer::emitWifiRSSI(AsyncEventSourceClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    String str = String(Network::getInstance()->rssi);
    sendEvent(str, "wifiRSSI", c);
}

void WebServer::emitBTConnected(AsyncEventSourceClient *c) {
    String str = (Bluetooth::getInstance()->state == Bluetooth::Connected) ? "true" : "false";
    sendEvent(str, "btConnected", c);
}

void WebServer::emitBTScannedDevices() {
    String json = "";
    StaticJsonDocument<1536> doc;
    Bluetooth* bt = Bluetooth::getInstance();
    for (int i = 0; i < Bluetooth::MaxScannedDevices; i++) {
        if (! bt->scannedDevices[i].used) continue;
        JsonObject d = doc.createNestedObject();
        d["name"] = bt->scannedDevices[i].device.name;
        d["address"] = bt->scannedDevices[i].device.address;
    }
    serializeJson(doc, json);
    sendEvent(json, "btScannedDevices");
}

void WebServer::emitBTConnectedDevice(AsyncEventSourceClient *c) {
    String json = "";
    StaticJsonDocument<128> doc;
    Bluetooth* bt = Bluetooth::getInstance();
    doc["name"] = bt->connectedDevice.name;
    doc["address"] = bt->connectedDevice.address;
    serializeJson(doc, json);
    sendEvent(json, "btConnectedDevice");

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
