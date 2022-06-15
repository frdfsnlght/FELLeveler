#ifndef NETWORK_H
#define NETWORK_H

enum WifiState {
    AP,
    Unconnected,
    Connecting,
    Connected
};

const char* AP_SSID = "FELLoader";
const char* AP_PASSWORD = "1234";

WifiState wifiState = AP;

void setupNetwork() {
    if (strcmp(config.wifiSSID, "") == 0) {
        wifiState = Unconnected;
        WiFi.mode(WIFI_STA);
    } else {
        wifiState = AP;
        Serial.print("Network starting AP ");
        Serial.print(AP_SSID);
        Serial.print(" with password ");
        Serial.println(AP_PASSWORD);
        WiFi.softAP(AP_SSID, AP_PASSWORD);
        Serial.print("Network IP Address: ");
        Serial.println(WiFi.softAPIP());
    }
    Serial.println("Network setup complete");
}

void loopNetwork() {
    if (wifiState == Unconnected) {
        Serial.print("Network connecting to ");
        Serial.print(config.wifiSSID);
        Serial.print(" with password ");
        Serial.println(config.wifiPassword);
        WiFi.begin(config.wifiSSID, config.wifiPassword);
        wifiState = Connecting;
    } else if (wifiState == Connecting) {
        if (WiFi.status() == WL_CONNECTED) {
            wifiState = Connected;
            Serial.println("Network connected");
            Serial.print("Network IP Address: ");
            Serial.println(WiFi.localIP());
        } else if (WiFi.status() == WL_CONNECT_FAILED) {
            wifiState = Unconnected;
            Serial.println("Network connection failed");
        }
    } else if (wifiState == Connected) {
        if (WiFi.status() == WL_CONNECTION_LOST) {
            wifiState = Unconnected;
            Serial.println("Network connection lost");
        }
    }
}

#endif