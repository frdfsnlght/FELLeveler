#include "network.h"

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <SPIFFS.h>
#include "config.h"
#include "secrets.h"

Network* Network::instance = nullptr;
const char* Network::APSSID = DEFAULT_AP_SSID;
const char* Network::APPassword = DEFAULT_AP_PASSWORD;
const int Network::OTAPort = 3232;
const char* Network::Hostname = "felleveler";
const int Network::MaxConnectionAttempts = 6;
const int Network::ConnectionAttemptInterval = 10000;
const IPAddress Network::APAddress(8, 8, 8, 8);
const IPAddress Network::APNetmask(255, 255, 255, 0);

Network* Network::getInstance() {
    if (instance == nullptr) instance = new Network();
    return instance;
}

void Network::setup() {
    Config* config = Config::getInstance();
    if (strcmp(config->wifiSSID, "") != 0) {
        state = Unconnected;
        WiFi.mode(WIFI_STA);
        connectionAttempts = 0;
    } else {
        setupAP();
    }

    // OTA
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
    ArduinoOTA.begin();

    Serial.println("Network and OTA setup complete");
}

void Network::loop() {
    Config* config = Config::getInstance();
    if (state == Unconnected) {
        Serial.print("Network connecting to \"");
        Serial.print(config->wifiSSID);
        Serial.print("\" with password \"");
        Serial.print(config->wifiPassword);
        Serial.println("\"");
        WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
        WiFi.setHostname(Hostname);
        WiFi.begin(config->wifiSSID, config->wifiPassword);
        state = Connecting;
        connectionAttempts++;
        stateChangedListeners.call();
    } else if (state == Connecting) {
        if (WiFi.status() == WL_CONNECTED) {
            state = Connected;
            Serial.println("Network connected");
            Serial.print("Network IP Address: ");
            ipAddress = WiFi.localIP();
            Serial.println(WiFi.localIP());
            stateChangedListeners.call();
        } else if (WiFi.status() == WL_CONNECT_FAILED) {
            Serial.println("Network connection failed");
            if (connectionAttempts >= MaxConnectionAttempts) {
                Serial.println("Too many failures, setting AP mode");
                setupAP();
            } else {
                Serial.println("Waiting for the next attempt");
                state = Waiting;
                lastConnectionAttemptTime = millis();
            }
            stateChangedListeners.call();
        }
    } else if (state == Waiting) {
        if ((millis() - lastConnectionAttemptTime) > ConnectionAttemptInterval) {
            state = Unconnected;
        }
    } else if (state == Connected) {
        if (WiFi.status() == WL_CONNECTION_LOST) {
            state = Unconnected;
            connectionAttempts = 0;
            lastConnectionAttemptTime = 0;
            Serial.println("Network connection lost");
            ipAddress = (uint32_t)0;
            stateChangedListeners.call();
        } else {
            if (WiFi.RSSI() != rssi) {
                rssi = WiFi.RSSI();
                wifiRSSIChangedListeners.call();
            }
        }
    } else if (state == AP) {
        dnsServer.processNextRequest();
    }
    ArduinoOTA.handle();
}

bool Network::hasNetwork() {
    return (state == AP) || (state == Connected);
}

void Network::setupAP() {
    state = AP;
    Serial.print("Network starting AP \"");
    Serial.print(APSSID);
    Serial.print("\" with password \"");
    Serial.print(APPassword);
    Serial.println("\"");
    WiFi.softAPConfig(APAddress, APAddress, APNetmask);
    WiFi.softAP(APSSID, APPassword);
    dnsServer.setErrorReplyCode(DNSReplyCode::NoError); 
    dnsServer.start(53, "*", APAddress);
    Serial.print("Network IP Address: ");
    Serial.println(WiFi.softAPIP());
    stateChangedListeners.call();
}