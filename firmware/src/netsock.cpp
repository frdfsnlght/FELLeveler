#include "netsock.h"

#include <Arduino.h>
#include "network.h"
#include "debug.h"
#include "leveler.h"

#ifdef DEBUG_NETSOCK
    #define DEBUG(msg) Serial.println(msg)
#else
    #define DEBUG(msg)
#endif

Netsock* Netsock::instance = nullptr;
const char* Netsock::StateStrings[] = {
    "Idle",
    "Connected",
    "WaitingForClient",
    "ConnectToServer",
    "WaitingToConnect"
};

Netsock* Netsock::getInstance() {
    if (instance == nullptr) instance = new Netsock();
    return instance;
}

void Netsock::setup() {
    led.blink();

    Config* config = Config::getInstance();
    state = Idle;
    remoteName[0] = '\0';
    remoteAddress.fromString("0.0.0.0");
    config->settingsChangedListeners.add([](void) {
        Netsock::getInstance()->handleSettingsChanged();
    });

    Leveler* leveler = Leveler::getInstance();
    leveler->rollChangedListeners.add([](void) {
        instance->sendMeasurements();
    });
    leveler->pitchChangedListeners.add([](void) {
        instance->sendMeasurements();
    });

    sendBufferStart = sendBufferEnd = 0;

    Network::getInstance()->stateChangedListeners.add([](void) {
        instance->handleNetworkStateChanged();
    });

    Serial.println("Netsock setup complete");
}

void Netsock::loop() {
    led.loop();
    //if (! Network::getInstance()->hasNetwork()) return;

    if (mode == Config::Tractor) {
        if (state == WaitingForClient && server.available()) {
            client = server.accept();
            remoteAddress = client.remoteIP();
            doConnect();
        } else if ((state == Connected) && (! client.connected())) {
            client.stop();
            doDisconnect(true);
        }
    } else if (mode == Config::Implement) {
        if (state == ConnectToServer) {
            Config* config = Config::getInstance();
            connectionAttempts++;
            Serial.printf("Netsock connecting to tractor at %s, port %d\n", config->tractorAddress.toString(), Port);
            if (client.connect(config->tractorAddress, Port)) {
                remoteAddress = config->tractorAddress;
                doConnect();
            } else {
                Serial.printf("Netsock connection failed attempt %d\n", connectionAttempts);
                lastConnectionAttemptTime = millis();
                setState(WaitingToConnect);
            }
        } else if (state == WaitingToConnect) {
            if ((millis() - lastConnectionAttemptTime) > ConnectionAttemptInterval) {
                setState(ConnectToServer);
            }
        } else if ((state == Connected) && (! client.connected())) {
            client.stop();
            doDisconnect(true);
        }
    }

    if ((state == Connected) && client.connected()) {
        while (client.available()) {
            int ch = client.read();
            switch (ch) {
                case '\r': break;
                case '\n':
                    receiveBuffer[receiveBufferPos] = '\0';
                    processReceiveBuffer();
                    resetReceiveBuffer();
                    break;
                default:
                    receiveBuffer[receiveBufferPos++] = ch;
                    break;
            }
        }

        while (client.connected() && (sendBufferStart != sendBufferEnd)) {
            if (client.write(sendBuffer[sendBufferStart])) {
                sendBufferStart++;
                if (sendBufferStart == MaxSendBuffer)
                    sendBufferStart = 0;
            } else
                break;
        }
    }
}

void Netsock::setState(State newState) {
    if (state == newState) return;
    state = newState;
#ifdef DEBUG_NETSOCK
    Serial.printf("Netsock state changed to %s\n", StateStrings[state]);
#endif
    stateChangedListeners.call();
}

void Netsock::setupNetsock() {
    Config* config = Config::getInstance();

    if (state == Connected) {
        client.stop();
        doDisconnect(false);
    }

    mode = config->mode;

    if (mode == Config::Tractor)  {
        Serial.println("Netsock setup for tractor");
        setupServer();
    } else if (mode == Config::Implement) {
        Serial.println("Netsock setup for implement");
        setupClient();
    } else {
        Serial.printf("Netsock cannot be setup, unknown mode (%d)\n", mode);
        setState(Idle);
    }
}

void Netsock::setupServer() {
    Serial.println("Netsock server starting");
    server = WiFiServer(Port);
    server.begin();
    setState(WaitingForClient);
}

void Netsock::setupClient() {
    client = WiFiClient();
    connectionAttempts = 0;
    setState(ConnectToServer);
}

void Netsock::doConnect() {
    resetReceiveBuffer();
    resetSendBuffer();
    led.turnOn();
    remoteAddress = client.remoteIP();
    if (mode == Config::Tractor) {
        Serial.printf("Netsock connected to implement at %s\n", remoteAddress.toString());
    } else if (mode == Config::Implement) {
        Serial.printf("Netsock connected to tractor at %s\n", remoteAddress.toString());
        connectionAttempts = 0;
    }
    setState(Connected);
    sendDeviceInfo();
}

void Netsock::doDisconnect(bool setNextState) {
    led.blink();
    if (mode == Config::Tractor) {
        Serial.println("Netsock disconnected from implement");
        if (setNextState) setState(WaitingForClient);
        measurements.roll = 0;
        measurements.pitch = 0;
        measurementsChangedListeners.call();
    } if (mode == Config::Implement) {
        Serial.println("Netsock disconnected from tractor");
        if (setNextState) setState(ConnectToServer);
    }
    remoteName[0] = '\0';
    remoteAddress.fromString("0.0.0.0");
    remoteDeviceChangedListeners.call();
}

void Netsock::resetReceiveBuffer() {
    receiveBufferPos = 0;
    receiveBuffer[0] = '\0';
}

void Netsock::resetSendBuffer() {
    sendBufferStart = sendBufferEnd = 0;
}

void Netsock::processReceiveBuffer() {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, receiveBuffer);
    if (err) {
        Serial.print("BT recieved bad JSON (");
        Serial.print(err.c_str());
        Serial.print("): ");
        Serial.println(receiveBuffer);
        return;
    }
    // device info can be received by both tractor and implement
    if (doc.containsKey("name")) {
        setRemoteDevice(doc);
        return;
    }
    if (mode == Config::Tractor) {
        if (doc.containsKey("pitch")) {
            setMeasurements(doc);
            return;
        }
    }
    Serial.print("Unknown JSON document received: ");
    Serial.println(receiveBuffer);
}

void Netsock::setRemoteDevice(const JsonDocument& doc) {
    strcpy(remoteName, doc["name"]);
#ifdef DEBUG_NETSOCK
    Serial.printf("Netsock connected to %s\n", remoteName);
#endif
    remoteDeviceChangedListeners.call();
}

void Netsock::setMeasurements(const JsonDocument& doc) {
    measurements.roll = doc["roll"];
    measurements.pitch = doc["pitch"];
    measurementsChangedListeners.call();
}

void Netsock::sendString(String str) {
    const char* ptr = str.c_str();
    bool end = false;
    char ch;
    while (! end) {
        ch = *ptr;
        ptr++;
        if (ch == '\0') {
            ch = '\n';
            end = true;
        }
        if (! sendChar(ch)) {
            Serial.println("Netsock send buffer overflow!");
            return;
        }
    }
}

bool Netsock::sendChar(const char ch) {
    sendBuffer[sendBufferEnd++] = ch;
    if (sendBufferEnd == MaxSendBuffer)
        sendBufferEnd = 0;
    return sendBufferStart != sendBufferEnd;
}

void Netsock::sendDeviceInfo() {
    if (state != Connected) return;
    String json = "";
    StaticJsonDocument<128> doc;
    doc["name"] = Config::getInstance()->name;
    serializeJson(doc, json);
    sendString(json);
}

void Netsock::sendMeasurements() {
    if (state != Connected) return;
    if (mode != Config::Implement) return;
    String json = "";
    Leveler* leveler = Leveler::getInstance();
    StaticJsonDocument<128> doc;
    doc["roll"] = leveler->roll;
    doc["pitch"] = leveler->pitch;
    serializeJson(doc, json);
    sendString(json);
}

void Netsock::handleSettingsChanged() {
    Config* config = Config::getInstance();
    if (config->mode != mode)
        setupNetsock();
}

void Netsock::handleNetworkStateChanged() {
    Network* network = Network::getInstance();
    if (network->state == Network::Connected) {
        setupNetsock();

    } else if ((network->state == Network::Disconnect) || (network->state == Network::OTA)) {
        client.stop();
        doDisconnect(false);
        setState(Idle);

    }
}
