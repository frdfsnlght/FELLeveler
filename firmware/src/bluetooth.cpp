#include "bluetooth.h"

//#include <Arduino.h>
#include "config.h"

Bluetooth* Bluetooth::instance = nullptr;
const String Bluetooth::BTBaseName = String("FEL-");

Bluetooth* Bluetooth::getInstance() {
    if (instance == nullptr) instance = new Bluetooth();
    return instance;
}

void Bluetooth::setup() {
    led.blink();
    Config* config = Config::getInstance();
    String devName = BTBaseName + config->name;
    master = config->mode == Config::Tractor;
    connected = false;
    bt.begin(devName, master);
    Serial.println("Bluetooth setup complete");
}

void Bluetooth::loop() {
    led.loop();
/*
    if (bt.connected() && (! connected)) {
        connected = true;
        led.turnOn();
        if (master) {
            bt.print("\n\n\nCONNECTED\n");
        }
    } else if ((! bt.connected()) && connected) {
        connected = false;
        led.blink();
    }
*/
}

void Bluetooth::scanDevices() {
    // TODO
}

bool Bluetooth::canPairDevice() {
    // TODO
}

void Bluetooth::pairDevice(const char* address) {
    // TODO
}

bool Bluetooth::canUnpairDevice(const char* address) {
    // TODO
}

void Bluetooth::unpairDevice(const char* address) {
    // TODO
}

void Bluetooth::unpair() {
    // TODO
}

void Bluetooth::startPairing() {
    // TODO
}

void Bluetooth::stopPairing() {
    // TODO
}

void Bluetooth::handleCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT: // When SPP mode is initialized
            Serial.println("BT initialized"); break;
        case ESP_SPP_UNINIT_EVT: // When the SPP mode is deinitialized
            Serial.println("BT uninitialized"); break;
        case ESP_SPP_DISCOVERY_COMP_EVT: // When service discovery is complete
            Serial.println("BT discovery completed"); break;
        case ESP_SPP_OPEN_EVT: // When an SPP client opens a connection
            Serial.println("BT opened connection"); break;
        case ESP_SPP_CLOSE_EVT: // When an SPP connection is closed
            Serial.println("BT closed connection"); break;
        case ESP_SPP_START_EVT: // When the SPP server is initialized
            Serial.println("BT server started"); break;
        case ESP_SPP_CL_INIT_EVT: // When an SPP client initializes a connection
            Serial.println("BT client initialized"); break;
        case ESP_SPP_DATA_IND_EVT: // When receiving data through an SPP connection
            receiveData(); break;
        case ESP_SPP_CONG_EVT: // When congestion status changes on an SPP connection
            Serial.println("BT congestion"); break;
        case ESP_SPP_WRITE_EVT: // When sending data through SPP.
            break;
        case ESP_SPP_SRV_OPEN_EVT: // When a client connects to the SPP server
            Serial.println("BT client connected"); break;
        case ESP_SPP_SRV_STOP_EVT: // When the SPP server stops
            Serial.println("BT server stopped"); break;
        default:
            break;
    }
}

void Bluetooth::receiveData() {
    while (bt.available()) {
        int ch = bt.read();
    }
}