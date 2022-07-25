#include "network.h"

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>
#include "debug.h"
#include "secrets.h"

#ifdef DEBUG_NETWORK
#define DEBUG(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

Network* Network::instance = nullptr;
const int Network::OTAPort = 3232;
const char* Network::Hostname = "felleveler";
const int Network::MaxConnectionAttempts = 6;
const int Network::ConnectionAttemptInterval = 10000;
//const IPAddress Network::APAddress(8, 8, 8, 8);
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
    config->settingsChangedListeners.add([](void) {
        instance->handleSettingsChanged();
    });

    setupDNSServer();
    setupOTA();
    setupWebServer();
    setupSockIO();
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
                setupAP(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
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
            int newRSSI = WiFi.RSSI();
            if (newRSSI != rssi) {
                rssi = newRSSI;
                wifiRSSIChangedListeners.call();
            }
        }
    }

    if (available()) {
        if (state == AP)
            dnsServer.processNextRequest();
        ArduinoOTA.handle();
        sockio.loop();
    }
}

bool Network::available() {
    return (state == AP) || (state == Connected);
}

void Network::handleSettingsChanged() {
    Config* config = Config::getInstance();
    if ((config->mode != mode) || (config->wifiMode != wifiMode))
        setupWifi();
}

void Network::setState(State newState) {
    if (newState == state) return;
    state = newState;
    Serial.printf("Network state changed to %s\n", StateStrings[state]);
    stateChangedListeners.call();
}

void Network::setupWifi() {
    Config* config = Config::getInstance();

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

    mode = config->mode;
    wifiMode = config->wifiMode;

    if (mode == Config::Tractor)  {
        if (wifiMode == Config::TractorWifi) {
            Serial.println("Network setup for tractor AP");
            setupAP(config->tractorSSID, config->tractorPassword);
        } else if (wifiMode == Config::HouseWifi) {
            if (strlen(config->houseSSID) == 0) {
                Serial.println("Network setup for default AP");
                setupAP(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
            } else {
                Serial.println("Network setup for house station");
                setupStation(config->houseSSID, config->housePassword);
            }
        }
    } else if (mode == Config::Implement) {
        if (wifiMode == Config::TractorWifi) {
            if (strlen(config->tractorSSID) == 0) {
                Serial.println("Network setup for default AP");
                setupAP(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
            } else {
                Serial.println("Network setup for tractor station");
                setupStation(config->tractorSSID, config->tractorPassword);
            }
        } else if (wifiMode == Config::HouseWifi) {
            if (strlen(config->houseSSID) == 0) {
                Serial.println("Network setup for default AP");
                setupAP(DEFAULT_AP_SSID, DEFAULT_AP_PASSWORD);
            } else {
                Serial.println("Network setup for house station");
                setupStation(config->houseSSID, config->housePassword);
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
    IPAddress address = Config::getInstance()->tractorAddress;
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
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
}

void Network::setupWebServer() {
    webServer.serveStatic("/", SPIFFS, "/w/");
}

void Network::setupSockIO() {
    sockio.on("connected", [](SockIOServerClient& client) {
        Serial.println("Network websocket server connected");
        client.on("test", [](SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
            instance->apiTest(client, args, ret);
        });
    });
}

void Network::activateServices() {
    if (state == AP)
        dnsServer.start(53, "*", Config::getInstance()->tractorAddress);
    ArduinoOTA.begin();
    webServer.begin();
    sockio.begin();
}

void Network::deactivateServices() {
    sockio.end();
    webServer.stop();
    ArduinoOTA.end();
    if (state == AP)
        dnsServer.stop();

}

// ======================================================
// SockIO API

void Network::apiTest(SockIOServerClient& client, JsonArray& args, JsonArray& ret) {
    ret.add(true);
    ret.add("a string");
    ret.add(3.1415);
    ret.add(1234);
}
