#include "network.h"

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>

#include "secrets.h"
#include "leveler.h"

Network* Network::instance = nullptr;
const int Network::OTAPort = 3232;
const char* Network::Hostname = "felleveler";
const int Network::MaxConnectionAttempts = 6;
const int Network::ConnectionAttemptInterval = 10000;
const int Network::ReportRSSIInterval = 1000;
const IPAddress Network::APNetmask(255, 255, 255, 0);

const char* Network::StateStrings[] = {
    "Idle",
    "OTA",
    "AP",
    "Disconnect",
    "Connect",
    "Connecting",
    "Waiting",
    "Connected"
};

Network* Network::getInstance() {
    if (instance == nullptr) instance = new Network();
    return instance;
}

void Network::setup() {
    led.setup();
    led.turnOn();
    Config* config = Config::getInstance();
    state = Idle;
    setupWifi();
/* TODO remove
    config->settingsListeners.add([](void) {
        instance->handleSettingsChanged();
    });
*/

    setupDNSServer();
    setupOTA();
    setupWebServer();
    setupWebSockIO();
    Serial.println("Network setup complete");
    led.blink();
}

void Network::loop() {
    led.loop();
    if (state == Connect) {
        connectionAttempts++;
        Serial.printf("Connecting to \"%s\", attempt %d of %d\n", ssid, connectionAttempts, MaxConnectionAttempts);
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(Hostname);
        WiFi.begin(ssid, password);
        setState(Connecting);
    } else if (state == Connecting) {
        if (WiFi.status() == WL_CONNECTED) {
            ipAddress = WiFi.localIP();
            Serial.println("Network connected");
            Serial.printf("Network IP Address: %s\n", ipAddress.toString());
            Serial.println("Network OTA started");
            setState(Connected);
            activateServices();
        } else if (WiFi.status() == WL_CONNECT_FAILED) {
            Serial.println("Network connection failed");
            if (connectionAttempts >= MaxConnectionAttempts) {
                Serial.println("Network station failure, setup default AP");
                setupAP(DEFAULT_TRACTOR_SSID, DEFAULT_TRACTOR_PASSWORD);
            } else {
                Serial.println("Network waiting for next connection attempt");
                lastConnectionAttemptTime = millis();
                setState(Waiting);
            }
        }
    } else if (state == Waiting) {
        if ((millis() - lastConnectionAttemptTime) > ConnectionAttemptInterval) {
            setState(Connect);
        }
    } else if (state == Connected) {
        if (WiFi.status() == WL_CONNECTION_LOST) {
            Serial.println("Network connection lost");
            setState(Disconnect);
            connectionAttempts = 0;
            lastConnectionAttemptTime = 0;
            ipAddress = (uint32_t)0;
            setState(Connect);
        } else {
            static unsigned long lastSampleTime = 0;
            if ((millis() - lastSampleTime) > ReportRSSIInterval) {
                lastSampleTime = millis();
                int newRSSI = WiFi.RSSI();
                if (newRSSI != rssi) {
                    rssi = newRSSI;
                    wifiRSSIListeners.call();
                }
            }
        }
    }

    if (available()) {
        if (state == AP)
            dnsServer.processNextRequest();
        ArduinoOTA.handle();
        webServer.handleClient();
        webSock.loop();
        if (implSock) implSock->loop();
        if (implClient) implClient->loop();
    }
}

bool Network::available() {
    return (state == AP) || (state == Connected);
}

/*
void Network::handleSettingsChanged() {
    Config* config = Config::getInstance();
    if ((config->mode != mode) || (config->wifiMode != wifiMode))
        setupWifi();
}
*/

void Network::setState(State newState) {
    if (newState == state) return;
    state = newState;
    Serial.printf("Network state changed to %s\n", StateStrings[state]);
    stateListeners.call();
}

void Network::setupWifi() {
    Config* config = Config::getInstance();

/*
    if (state == Connected) {
        deactivateServices();
        setState(Disconnect);
        WiFi.disconnect();
        setState(Idle);
    } else if (state == AP) {
        deactivateServices();
        setState(Disconnect);
        WiFi.softAPdisconnect();
        setState(Idle);
    }
*/

    mode = config->running.mode;
    wifiMode = config->running.wifiMode;

    if (mode == Config::Tractor)  {
        if (wifiMode == Config::TractorWifi) {
            Serial.println("Network setup for tractor AP");
            setupAP(config->running.tractorSSID, config->running.tractorPassword);
        } else if (wifiMode == Config::HouseWifi) {
            if (strlen(config->running.houseSSID) == 0) {
                Serial.println("Network setup for default AP");
                setupAP(DEFAULT_TRACTOR_SSID, DEFAULT_TRACTOR_PASSWORD);
            } else {
                Serial.println("Network setup for house station");
                setupStation(config->running.houseSSID, config->running.housePassword);
            }
        }
    } else if (mode == Config::Implement) {
        if (wifiMode == Config::TractorWifi) {
            if (strlen(config->running.tractorSSID) == 0) {
                Serial.println("Network setup for default AP");
                setupAP(DEFAULT_TRACTOR_SSID, DEFAULT_TRACTOR_PASSWORD);
            } else {
                Serial.println("Network setup for tractor station");
                setupStation(config->running.tractorSSID, config->running.tractorPassword);
            }
        } else if (wifiMode == Config::HouseWifi) {
            if (strlen(config->running.houseSSID) == 0) {
                Serial.println("Network setup for default AP");
                setupAP(DEFAULT_TRACTOR_SSID, DEFAULT_TRACTOR_PASSWORD);
            } else {
                Serial.println("Network setup for house station");
                setupStation(config->running.houseSSID, config->running.housePassword);
            }
        }
    } else {
        Serial.printf("Network cannot be setup, unknown mode (%d) or wifiMode (%d)\n", mode, wifiMode);
        setState(Idle);
    }

}

void Network::setupAP(const char* apSSID, const char* apPassword) {
    Serial.printf("Network starting AP with SSID \"%s\" and password \"%s\"\n", apSSID, apPassword);
    strcpy(ssid, apSSID);
    strcpy(password, apPassword);
    IPAddress address = Config::getInstance()->running.tractorAddress;
    WiFi.softAPConfig(address, address, APNetmask);
    WiFi.softAP(apSSID, apPassword);
    Serial.print("Network IP Address: ");
    Serial.println(WiFi.softAPIP());
    setState(AP);
    activateServices();
}

void Network::setupStation(const char* stationSSID, const char* stationPassword) {
    Serial.printf("Network preparing to connect to SSID \"%s\" with password \"%s\"\n", stationSSID, stationPassword);
    strcpy(ssid, stationSSID);
    strcpy(password, stationPassword);
    connectionAttempts = 0;
    lastConnectionAttemptTime = 0;
    setState(Connect);
}

void Network::setupDNSServer() {
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError); 
}

void Network::setupOTA() {
    ArduinoOTA.setPort(OTAPort);
    ArduinoOTA.setHostname(Hostname);
    ArduinoOTA.setRebootOnSuccess(true);
  
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    ArduinoOTA.onStart([]() {
        if (ArduinoOTA.getCommand() == U_FLASH)
            Serial.println("OTA Update flash");
        else {
            Serial.println("OTA Update filesystem");
            SPIFFS.end();
        }
        Network::getInstance()->setState(OTA);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nReboot");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
}

void Network::setupWebServer() {
    webServer.on("/test", []() {
        instance->webServer.send(200, "text/plain", "test worked!");
        return true;
    });
    webServer.serveStatic("/", SPIFFS, "/w/");
}

void Network::setupWebSockIO() {
    webSock.on("connected", [](SockIOServerClient& client) {
        Serial.println("Network webSock server connected");
        client.on("test", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiWebTest(client, args, ret);
        });
        client.on("configure", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiWebConfigure(client, args, ret);
        });
        client.on("calibrateLevel", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiWebCalibrateLevel(client, args, ret);
        });
        client.on("calibrateTipped", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiWebCalibrateTipped(client, args, ret);
        });
        client.on("saveConfig", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiWebSaveConfig(client, args, ret);
        });
        client.on("reboot", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiWebReboot(client, args, ret);
        });

        instance->emitWebConfigDirty(&client);
        instance->emitWebConfigSettings(&client, true);
        instance->emitWebConfigSettings(&client, false);
        instance->emitWebConfigCalibrated(&client);
        if (instance->state == Connected)
            instance->emitWebWifiRSSI(&client);
        instance->emitWebAngles(&client);
        instance->emitWebRemoteConnected(&client);
        instance->emitWebRemoteInfo(&client);
        if (instance->mode == Config::Tractor)
            instance->emitWebRemoteAngles(&client);
    });

    // Subscribe to listeners

    Config* config = Config::getInstance();
    Leveler* leveler = Leveler::getInstance();

    wifiRSSIListeners.add([](void) {
        instance->emitWebWifiRSSI(NULL);
    });
    config->dirtyListeners.add([](void) {
        instance->emitWebConfigDirty(NULL);
    });
    config->settingsListeners.add([](void) {
        instance->emitWebConfigSettings(NULL, false);
        instance->emitImplRemoteInfo(NULL);
    });
    config->calibratedListeners.add([](void) {
        instance->emitWebConfigCalibrated(NULL);
    });

    leveler->anglesListeners.add([](void) {
        instance->emitWebAngles(NULL);
        instance->emitImplRemoteAngles(NULL);
    });
    leveler->remoteConnectedListeners.add([](void) {
        instance->emitWebRemoteConnected(NULL);
    });
    leveler->remoteInfoListeners.add([](void) {
        instance->emitWebRemoteInfo(NULL);
    });
    leveler->remoteAnglesListeners.add([](void) {
        instance->emitWebRemoteAngles(NULL);
    });
}

void Network::setupImplSockIO() {
    if (mode == Config::Tractor) {
        implSock = new SockIOServer(82, "/");
        implSock->on("connected", [](SockIOServerClient& client) {
            client.on("connected", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
                Serial.println("Network implSocket server connected");
                instance->led.turnOn();
                Leveler* leveler = Leveler::getInstance();
                leveler->setRemoteConnected(true);
                instance->emitImplRemoteInfo(&client);
            });
            client.on("disconnected", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
                Serial.println("Network implSocket server disconnected");
                instance->led.blink();
                Leveler* leveler = Leveler::getInstance();
                leveler->setRemoteConnected(false);
            });
            client.on("remoteInfo", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
                instance->apiImplRemoteInfo(args, ret);
            });
            client.on("remoteAngles", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
                instance->apiImplRemoteAngles(args, ret);
            });
        });
        implSock->begin();

    } else if (mode == Config::Implement) {
        implClient = new SockIOClient(Config::getInstance()->running.tractorAddress.toString(), 82, "/");
        implClient->on("connected", [](JsonArray& args, JsonArray& ret) {
            Serial.println("Network implClient connected");
            instance->led.turnOn();
            Leveler* leveler = Leveler::getInstance();
            leveler->setRemoteConnected(true);
            instance->emitImplRemoteInfo(NULL);
        });
        implClient->on("disconnected", [](JsonArray& args, JsonArray& ret) {
            Serial.println("Network implClient disconnected");
            instance->led.blink();
            Leveler* leveler = Leveler::getInstance();
            leveler->setRemoteConnected(false);
        });
        implClient->on("remoteInfo", [](JsonArray& args, JsonArray& ret) {
            instance->apiImplRemoteInfo(args, ret);
        });
        implClient->begin();
    }
}

void Network::teardownImplSockIO() {
    if (implSock) {
        implSock->end();
        delete implSock;
        implSock = NULL;
    }
    if (implClient) {
        implClient->end();
        delete implClient;
        implClient = NULL;
    }
}

void Network::activateServices() {
    Serial.println("Network services activating");
    if (state == AP)
        dnsServer.start(53, "*", Config::getInstance()->running.tractorAddress);
    ArduinoOTA.begin();
    webServer.begin();
    webSock.begin();
    setupImplSockIO();
}

void Network::deactivateServices() {
    Serial.println("Network services deactivating");
    teardownImplSockIO();
    log_d("tore down implSockIO");
    webSock.end();
    log_d("webSock ended");
    webServer.stop();
    log_d("webServer stopped");
    ArduinoOTA.end();
    log_d("OTA ended");
    if (state == AP) {
        dnsServer.stop();
        log_d("dnsServer stopped");
    }
}

// ======================================================
// SockIO Web API

void Network::apiWebTest(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    ret.add(true);
    ret.add("a string");
    ret.add(3.1415);
    ret.add(1234);
}

void Network::apiWebConfigure(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    log_d("API configure");
    if ((args.size() != 1) || (! args[0].is<JsonObject>())) {
        ret.add(false);
        ret.add("expected object");
        return;
    }
    JsonObject obj = args[0].as<JsonObject>();
    Config* config = Config::getInstance();
    config->setSettings(
        obj["mode"],
        obj["wifiMode"],
        obj["name"],
        obj["houseSSID"],
        obj["housePassword"],
        obj["tractorSSID"],
        obj["tractorPassword"],
        obj["tractorAddress"],
        obj["enableDisplay"]
    );
    ret.add(true);
}

void Network::apiWebCalibrateLevel(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    log_d("API calibrateLevel");
    Leveler* leveler = Leveler::getInstance();
    leveler->calibrateLevel();
    ret.add(true);
}

void Network::apiWebCalibrateTipped(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    log_d("API calibrateTipped");
    if ((args.size() != 1) || (! args[0].is<JsonObject>())) {
        ret.add(false);
        ret.add("expected object");
        return;
    }
    JsonObject obj = args[0].as<JsonObject>();
    Leveler* leveler = Leveler::getInstance();
    leveler->calibrateTipped(
        obj["invertPitchAxis"]
    );
    ret.add(true);
}

void Network::apiWebSaveConfig(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    log_d("API saveConfig");
    Config::getInstance()->write();
    ret.add(true);
}

void Network::apiWebReboot(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    log_d("API reboot");
    ESP.restart();
}

void Network::emitWeb(SockIOServerClient *c, const String& event, JsonArray& array  ) {
    if (! available()) return;
    if (c)
        c->emit(event, array);
    else
        webSock.emit(event, array);
}

void Network::emitWebConfigDirty(SockIOServerClient *c) {
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(Config::getInstance()->dirty);
    emitWeb(c, "configDirty", array);
}

void Network::emitWebConfigSettings(SockIOServerClient *c, bool running) {
    StaticJsonDocument<384> doc;
    JsonArray array = doc.to<JsonArray>();
    JsonObject obj = array.createNestedObject();
    Config* config = Config::getInstance();
    if (running) {
        obj["mode"] = Config::ModeStrings[config->running.mode];
        obj["wifiMode"] = Config::WifiModeStrings[config->running.wifiMode];
        obj["name"] = config->running.name;
        obj["houseSSID"] = config->running.houseSSID;
        obj["housePassword"] = config->running.housePassword;
        obj["tractorSSID"] = config->running.tractorSSID;
        obj["tractorPassword"] = config->running.tractorPassword;
        obj["tractorAddress"] = config->running.tractorAddress.toString();
        obj["enableDisplay"] = config->running.enableDisplay;
        emitWeb(c, "runningSettings", array);
    } else {
        obj["mode"] = Config::ModeStrings[config->save.mode];
        obj["wifiMode"] = Config::WifiModeStrings[config->save.wifiMode];
        obj["name"] = config->save.name;
        obj["houseSSID"] = config->save.houseSSID;
        obj["housePassword"] = config->save.housePassword;
        obj["tractorSSID"] = config->save.tractorSSID;
        obj["tractorPassword"] = config->save.tractorPassword;
        obj["tractorAddress"] = config->save.tractorAddress.toString();
        obj["enableDisplay"] = config->save.enableDisplay;
        emitWeb(c, "saveSettings", array);
    }
}

void Network::emitWebConfigCalibrated(SockIOServerClient *c) {
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(Config::getInstance()->calibrated);
    emitWeb(c, "calibrated", array);
}

void Network::emitWebWifiRSSI(SockIOServerClient *c) {
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(rssi);
    emitWeb(c, "wifiRSSI", array);
}

void Network::emitWebAngles(SockIOServerClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    Leveler* leveler = Leveler::getInstance();
    array.add(leveler->roll);
    array.add(leveler->pitch);
    emitWeb(c, "angles", array);
}

void Network::emitWebRemoteConnected(SockIOServerClient *c) {
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(Leveler::getInstance()->remoteConnected);
    emitWeb(c, "remoteConnected", array);
}

void Network::emitWebRemoteInfo(SockIOServerClient *c) {
    StaticJsonDocument<192> doc;
    JsonArray array = doc.to<JsonArray>();
    Leveler* leveler = Leveler::getInstance();
    array.add(leveler->remoteName);
    array.add(leveler->remoteAddress);
    emitWeb(c, "remoteInfo", array);
}

void Network::emitWebRemoteAngles(SockIOServerClient *c) {
    static unsigned long lastEmitTime = 0;
    if ((millis() - lastEmitTime) < 1000) return;
    lastEmitTime = millis();
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    Leveler* leveler = Leveler::getInstance();
    array.add(leveler->remoteRoll);
    array.add(leveler->remotePitch);
    emitWeb(c, "remoteAngles", array);
}

// ======================================================
// SockIO Implement API

void Network::apiImplRemoteInfo(JsonArray& args, JsonArray& ret) {
    if (args.size() != 2) return;
    Leveler* leveler = Leveler::getInstance();
    leveler->setRemoteInfo(args[0], args[1]);
}

void Network::apiImplRemoteAngles(JsonArray& args, JsonArray& ret) {
    if (mode != Config::Tractor) return;
    if (args.size() != 2) return;
    Leveler* leveler = Leveler::getInstance();
    leveler->setRemoteAngles(args[0], args[1]);
}

void Network::emitImpl(SockIOServerClient *c, const String& event, JsonArray& array  ) {
    if (! available()) return;
    if (c)
        c->emit(event, array);
    else if (implSock)
        implSock->emit(event, array);
    else if (implClient)
        implClient->emit(event, array);
}

void Network::emitImplRemoteInfo(SockIOServerClient *c) {
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    Config* config = Config::getInstance();
    array.add(config->running.name);
    array.add(ipAddress.toString());
    emitImpl(c, "remoteInfo", array);
}

void Network::emitImplRemoteAngles(SockIOServerClient *c) {
    if (mode != Config::Implement) return;
    StaticJsonDocument<64> doc;
    JsonArray array = doc.to<JsonArray>();
    Leveler* leveler = Leveler::getInstance();
    array.add(leveler->roll);
    array.add(leveler->pitch);
    emitImpl(c, "remoteAngles", array);
}
