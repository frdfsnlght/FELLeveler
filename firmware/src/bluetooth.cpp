#include "bluetooth.h"

#include <Arduino.h>
#include <esp_bt_device.h>
#include "debug.h"
#include "leveler.h"

#ifdef DEBUG_BLUETOOTH
    #define DEBUG(msg) Serial.println(msg)
#else
    #define DEBUG(msg)
#endif

Bluetooth* Bluetooth::instance = nullptr;
const char Bluetooth::BTBaseName[] = "FEL-";

Bluetooth* Bluetooth::getInstance() {
    if (instance == nullptr) instance = new Bluetooth();
    return instance;
}

void Bluetooth::setup() {
    led.blink();
    Config* config = Config::getInstance();
    strcpy(deviceName, BTBaseName);
    strcat(deviceName, config->name);
    state = Stopped;
    justConnected = justDisconnected = false;

    Leveler* leveler = Leveler::getInstance();
    leveler->rollChangedListeners.add([](void) {
        instance->sendMeasurements();
    });
    leveler->pitchChangedListeners.add([](void) {
        instance->sendMeasurements();
    });

    bt.register_callback([](esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
        Bluetooth::getInstance()->btCallback(event, param);
    });

    start();

    Serial.println("Bluetooth setup complete");
}

void Bluetooth::loop() {
    //connectedChangedListeners.callNow();
    //connectedDeviceChangedListeners.callNow();
    //scannedDevicesChangedListeners.callNow();
    //measurementsChangedListeners.callNow();

    led.loop();

    if (justConnected)
        doConnect();

    if (justDisconnected)
        doDisconnect();

    while (bt.available()) {
        int ch = bt.read();
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

    switch (state) {
        case Connected:
            break;
        case WaitingForMaster:
            break;
        case ScanForSlave:
            scanForSlave();
            break;
        case ScanningForSlave:
            break;
        case ConnectingToSlave:
            connectToSlave();
            break;
        case ScanningDevices:
            break;
        case ScanningComplete:
            completeDeviceScan();
            break;
    }
}

void Bluetooth::scanDevices() {
    if (state == Connected) {
        bt.disconnect();
        doDisconnect();
    }
    state = ScanningDevices;
    bt.discoverClear();
    bt.discoverAsync([](BTAdvertisedDevice* device) {
        Serial.print("BT discovered: ");
        Serial.print(device->getName().c_str());
        Serial.println(" (");
        Serial.print(device->getAddress());
        Serial.println(')');
    }, 10000);  // 10 seconds
}

void Bluetooth::scanForSlave() {
    if (state == Connected) {
        bt.disconnect();
        doDisconnect();
    }
    state = ScanningForSlave;
    bt.discoverClear();
    bt.discoverAsync([](BTAdvertisedDevice* device) {
        Bluetooth::getInstance()->checkFoundDevice(device);
    }, 10000);  // 10 seconds
}

void Bluetooth::checkFoundDevice(BTAdvertisedDevice* device) {
    if (state != ScanningForSlave) return;
    Config* config = Config::getInstance();
    for (int i = 0; i < Config::MaxPairedDevices; i++) {
        if (config->pairedDevices[i].used &&
            (strcmp(config->pairedDevices[i].device.address, device->getAddress().toString().c_str()) == 0)) {
            state = ConnectingToSlave;
            slaveAddress = device->getAddress();
            break;
        }
    }
}

void Bluetooth::connectToSlave() {
    bt.discoverAsyncStop();
    if (bt.connect(slaveAddress)) {
        justConnected = true;
    } else {
        state = ScanForSlave;
    }
}

void Bluetooth::completeDeviceScan() {
    state = ScanForSlave;
    BTScanResults* results = bt.getScanResults();
    for (int i = 0; i < MaxScannedDevices; i++) {
        if (i >= results->getCount()) {
            scannedDevices[i].used = false;
            continue;
        }
        BTAdvertisedDevice* device = results->getDevice(i);
        scannedDevices[i].used = true;
        char name[256]; // max length is 248 bytes
        strcpy(name, device->getName().c_str());
        name[31] ='\0'; // make sure it fits
        strcpy(scannedDevices[i].device.name, name);
        strcpy(scannedDevices[i].device.address, device->getAddress().toString().c_str());
    }
    scannedDevicesChangedListeners.call();
}

bool Bluetooth::pairDevice(const char* address) {
    Config* config = Config::getInstance();
    BTScanResults* results = bt.getScanResults();
    for (int i = 0; i < results->getCount(); i++) {
        BTAdvertisedDevice* device = results->getDevice(i);
        if (strcmp(address, device->getAddress().toString().c_str()) == 0)
            return config->addPairedDevice(device->getName().c_str(), address);
    }
    return false;
}

void Bluetooth::btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT: // When SPP mode is initialized
            DEBUG("BT initialized"); break;
        case ESP_SPP_UNINIT_EVT: // When the SPP mode is deinitialized
            DEBUG("BT uninitialized"); break;
        case ESP_SPP_DISCOVERY_COMP_EVT: // When service discovery is complete
            DEBUG("BT discovery completed");
            if (state == ScanningDevices)
                state = ScanningComplete;
            break;
        case ESP_SPP_OPEN_EVT: // When an SPP client opens a connection
            DEBUG("BT opened connection");
            break;
        case ESP_SPP_CLOSE_EVT: // When an SPP connection is closed
            DEBUG("BT closed connection");
            justDisconnected = true;
            break;
        case ESP_SPP_START_EVT: // When the SPP server is initialized
            DEBUG("BT server started"); break;
        case ESP_SPP_CL_INIT_EVT: // When an SPP client initializes a connection
            DEBUG("BT client initialized"); break;
        case ESP_SPP_DATA_IND_EVT: // When receiving data through an SPP connection
            break;
        case ESP_SPP_CONG_EVT: // When congestion status changes on an SPP connection
            DEBUG("BT congestion"); break;
        case ESP_SPP_WRITE_EVT: // When sending data through SPP.
            break;
        case ESP_SPP_SRV_OPEN_EVT: // When a client connects to the SPP server
            DEBUG("BT client connected");
            justConnected = true;
            break;
        case ESP_SPP_SRV_STOP_EVT: // When the SPP server stops
            DEBUG("BT server stopped"); break;
        default:
            break;
    }
}

void Bluetooth::start() {
    mode = Config::getInstance()->mode;
    if (mode == Config::Tractor) {
        DEBUG("Beginning BT in master mode");
        bt.begin(deviceName, true);
        state = ScanForSlave;
    } else if (mode == Config::Implement) {
        DEBUG("Beginning BT in slave mode");
        bt.begin(deviceName);
        state = WaitingForMaster;
    }
}

void Bluetooth::stop() {
    bt.end();
    State oldState = state;
    state = Stopped;
    if (oldState == Connected)
        connectedChangedListeners.call();
}

void Bluetooth::doConnect() {
    justConnected = false;
    resetReceiveBuffer();
    led.turnOn();
    state = Connected;
    if (mode == Config::Tractor) {
        Serial.println("BT connected to implement");
    } else if (mode == Config::Implement) {
        Serial.println("BT connected to tractor");
    }
    connectedChangedListeners.call();
    sendDeviceInfo();
}

void Bluetooth::doDisconnect() {
    justDisconnected = false;
    led.blink();
    if (mode == Config::Tractor) {
        Serial.println("BT disconnected from implement");
        state = ScanForSlave;
        measurements.roll = 0;
        measurements.pitch = 0;
        measurementsChangedListeners.call();
    } if (mode == Config::Implement) {
        Serial.println("BT disconnected from tractor");
        state = WaitingForMaster;
    }
    connectedChangedListeners.call();
    connectedDevice.name[0] = '\0';
    connectedDevice.address[0] = '\0';
    connectedChangedListeners.call();
}

void Bluetooth::resetReceiveBuffer() {
    receiveBufferPos = 0;
    receiveBuffer[0] = '\0';
}

void Bluetooth::processReceiveBuffer() {
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
    if (doc.containsKey("name"))
        setConnectedDevice(doc);

    if (mode == Config::Tractor) {
        if (doc.containsKey("pitch")) {
            setMeasurements(doc);
            return;
        }
    }
    Serial.print("Unknown JSON document received: ");
    Serial.println(receiveBuffer);
}

void Bluetooth::setConnectedDevice(const JsonDocument& doc) {
    strcpy(connectedDevice.name, doc["name"]);
    strcpy(connectedDevice.address, doc["address"]);
#ifdef DEBUG_BLUETOOTH
    Serial.print("BT connected to ");
    Serial.print(connectedDevice.name);
    Serial.print(" (");
    Serial.print(connectedDevice.address);
    Serial.println(')');
#endif
    connectedDeviceChangedListeners.call();
}

void Bluetooth::setMeasurements(const JsonDocument& doc) {
    measurements.roll = doc["roll"];
    measurements.pitch = doc["pitch"];
    measurementsChangedListeners.call();
}

void Bluetooth::sendString(String str) {
    const char* ptr = str.c_str();
    while (*ptr != '\0')
        bt.write(*ptr);
}

void Bluetooth::sendDeviceInfo() {
    if (state != Connected) return;
    String json = "";
    StaticJsonDocument<128> doc;

    const uint8_t* addr1 = esp_bt_dev_get_address();
    esp_bd_addr_t addr2;
    memcpy(addr2, addr1, ESP_BD_ADDR_LEN);
    BTAddress address = BTAddress(addr2);

    doc["name"] = Config::getInstance()->name;
    doc["address"] = address.toString();
    serializeJson(doc, json);
    sendString(json);
}

void Bluetooth::sendMeasurements() {
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
