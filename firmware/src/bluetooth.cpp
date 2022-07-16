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
    state = Idle;
    justConnected = justDisconnected = false;
    connectedDevice.name[0] = '\0';
    connectedDevice.address[0] = '\0';

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

    sendBufferStart = sendBufferEnd = 0;

    /*
    // This doesn't work because it needs to happen after bt_init and before first TX.
    if (esp_bredr_tx_power_set(ESP_PWR_LVL_P9,ESP_PWR_LVL_P9) != ESP_OK)
        Serial.println("Unable to set BT power level!");
        delay(100);
    }
    */

    esp_power_level_t min,max;
    esp_bredr_tx_power_get(&min, &max);
    Serial.print("BT power min ");
    Serial.print(powerLevelToString(min));
    Serial.print(", max ");
    Serial.println(powerLevelToString(max));

    mode = config->mode;
    if (mode == Config::Tractor) {
        DEBUG("Beginning BT in master mode");
        bt.begin(deviceName, true);
        setState(ScanForSlave);
    } else if (mode == Config::Implement) {
        DEBUG("Beginning BT in slave mode");
        bt.begin(deviceName);
        setState(WaitingForMaster);
    }

    Serial.println("Bluetooth setup complete");
}

const char* Bluetooth::powerLevelToString(esp_power_level_t pl) {
    switch (pl) {
        case ESP_PWR_LVL_N12: return "-12dbm";
        case ESP_PWR_LVL_N9: return "-9dbm";
        case ESP_PWR_LVL_N6: return "-6dbm";
        case ESP_PWR_LVL_N3: return "-3dbm";
        case ESP_PWR_LVL_N0: return "0dbm";
        case ESP_PWR_LVL_P3: return "+3dbm";
        case ESP_PWR_LVL_P6: return "+6dbm";
        case ESP_PWR_LVL_P9: return "+9dbm";
        default: return "unknown";
    }
}

void Bluetooth::loop() {
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

    while (bt.connected() && (sendBufferStart != sendBufferEnd)) {
        if (bt.write(sendBuffer[sendBufferStart])) {
            sendBufferStart++;
            if (sendBufferStart == MaxSendBuffer)
                sendBufferStart = 0;
        } else
            break;
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
            checkScanTime();
            break;
        case ConnectToSlave:
            connectToSlave();
            break;
        case ScanningDevices:
            checkScanTime();
            break;
        case ScanningComplete:
            completeDeviceScan();
            break;
    }
}

const char* Bluetooth::stateToString(State s) {
    switch (s) {
        case Idle: return "Idle";
        case Connected: return "Connected";
        case WaitingForMaster: return "WaitingForMaster";
        case ScanForSlave: return "ScanForSlave";
        case ScanningForSlave: return "ScanningForSlave";
        case ConnectToSlave: return "ConnectToSlave";
        case ScanningDevices: return "ScanningDevices";
        case ScanningComplete: return "ScanningComplete";
        default: return "unknown";
    }
}

void Bluetooth::setState(State newState) {
    if (state == newState) return;
    state = newState;
#ifdef DEBUG_BLUETOOTH
    Serial.print("BT state is now ");
    Serial.println(stateToString(state));
#endif
    stateChangedListeners.call();
}

void Bluetooth::scanDevices() {
    bt.disconnect();
    if (state == Connected)
        doDisconnect();
    bt.discoverAsyncStop();
    setState(ScanningDevices);
    Serial.println("BT discovering devices");
    bt.discoverClear();
    startScanTime = millis();
    if (! bt.discoverAsync([](BTAdvertisedDevice* device) {
        Serial.print("BT discovered: ");
        Serial.print(device->getName().c_str());
        Serial.print(" (");
        Serial.print(device->getAddress().toString().c_str());
        Serial.print("), ");
        Serial.println(device->getRSSI());
    })) {
        Serial.println("BT discover failed!");
        setState(ScanForSlave);
    };
}

void Bluetooth::scanForSlave() {
    bt.disconnect();
    if (state == Connected)
        doDisconnect();
    bt.discoverAsyncStop();
    setState(ScanningForSlave);
    Serial.println("BT scanning for slaves");
    bt.discoverClear();
    startScanTime = millis();
    if (! bt.discoverAsync([](BTAdvertisedDevice* device) {
        Bluetooth::getInstance()->checkFoundDevice(device);
    })) {
        Serial.println("BT scan failed!");
        setState(ScanForSlave);
    }
}

void Bluetooth::checkScanTime() {
    if ((millis() - startScanTime) > MaxScanTime) {
        if (state == ScanningDevices)
            setState(ScanningComplete);
        else if (state == ScanningForSlave)
            setState(ScanForSlave);
        else {
            startScanTime = millis();
            Serial.println("BT scan is complete, but I don't know why?");
        }
    }
}

void Bluetooth::checkFoundDevice(BTAdvertisedDevice* device) {
    if (state != ScanningForSlave) return;
    Config* config = Config::getInstance();
    for (int i = 0; i < Config::MaxPairedDevices; i++) {
        if (config->pairedDevices[i].used &&
            (strcmp(config->pairedDevices[i].device.address, device->getAddress().toString().c_str()) == 0)) {
            slaveAddress = device->getAddress();
            setState(ConnectToSlave);
#ifdef DEBUG_BLUETOOTH
            Serial.print("BT found paired device ");
            Serial.println(slaveAddress.toString().c_str());
#endif
            break;
        }
    }
}

void Bluetooth::connectToSlave() {
    bt.discoverAsyncStop();
    Serial.print("BT attempting to connect to slave at ");
    Serial.println(slaveAddress.toString().c_str());
    if (bt.connect(slaveAddress)) {
        justConnected = true;
    } else {
        Serial.println("BT connection failed!");
        setState(ScanForSlave);
    }
}

void Bluetooth::completeDeviceScan() {
    bt.discoverAsyncStop();
    int count = 0;
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
        name[BTDevice::MaxBTNameLength - 1] ='\0'; // make sure it fits
        strcpy(scannedDevices[i].device.name, name);
        strcpy(scannedDevices[i].device.address, device->getAddress().toString().c_str());
        count++;
    }
    Serial.print("BT device discovery complete, found ");
    Serial.print(count);
    Serial.println(" devices");
    setState(ScanForSlave);
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
            DEBUG("ESP_SPP_INIT_EVT"); break;
        case ESP_SPP_UNINIT_EVT: // When the SPP mode is deinitialized
            DEBUG("ESP_SPP_UNINIT_EVT"); break;
        case ESP_SPP_DISCOVERY_COMP_EVT: // When service discovery is complete
            DEBUG("ESP_SPP_DISCOVERY_COMP_EVT");
            if (state == ScanningDevices)
                setState(ScanningComplete);
            else if (state == ScanningForSlave)
                setState(ScanForSlave);
            break;
        case ESP_SPP_OPEN_EVT: // When an SPP client opens a connection
            DEBUG("ESP_SPP_OPEN_EVT");
            break;
        case ESP_SPP_CLOSE_EVT: // When an SPP connection is closed
            DEBUG("ESP_SPP_CLOSE_EVT");
            justDisconnected = true;
            break;
        case ESP_SPP_START_EVT: // When the SPP server is initialized
            DEBUG("ESP_SPP_START_EVT"); break;
        case ESP_SPP_CL_INIT_EVT: // When an SPP client initializes a connection
            DEBUG("ESP_SPP_CL_INIT_EVT"); break;
        case ESP_SPP_DATA_IND_EVT: // When receiving data through an SPP connection
            break;
        case ESP_SPP_CONG_EVT: // When congestion status changes on an SPP connection
            DEBUG("ESP_SPP_CONG_EVT"); break;
        case ESP_SPP_WRITE_EVT: // When sending data through SPP.
            break;
        case ESP_SPP_SRV_OPEN_EVT: // When a client connects to the SPP server
            DEBUG("ESP_SPP_SRV_OPEN_EVT");
            justConnected = true;
            break;
        case ESP_SPP_SRV_STOP_EVT: // When the SPP server stops
            DEBUG("ESP_SPP_SRV_STOP_EVT"); break;
        default:
            break;
    }
}

void Bluetooth::doConnect() {
    justConnected = false;
    resetReceiveBuffer();
    led.turnOn();
    if (mode == Config::Tractor) {
        Serial.println("BT connected to implement");
    } else if (mode == Config::Implement) {
        Serial.println("BT connected to tractor");
    }
    setState(Connected);
    sendDeviceInfo();
}

void Bluetooth::doDisconnect() {
    justDisconnected = false;
    led.blink();
    if (mode == Config::Tractor) {
        Serial.println("BT disconnected from implement");
        setState(ScanForSlave);
        measurements.roll = 0;
        measurements.pitch = 0;
        measurementsChangedListeners.call();
    } if (mode == Config::Implement) {
        Serial.println("BT disconnected from tractor");
        setState(WaitingForMaster);
    }
    connectedDevice.name[0] = '\0';
    connectedDevice.address[0] = '\0';
    connectedDeviceChangedListeners.call();
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
    if (doc.containsKey("name")) {
        setConnectedDevice(doc);
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
            Serial.println("BT send buffer overflow!");
            return;
        }
    }
}

bool Bluetooth::sendChar(const char ch) {
    sendBuffer[sendBufferEnd++] = ch;
    if (sendBufferEnd == MaxSendBuffer)
        sendBufferEnd = 0;
    return sendBufferStart != sendBufferEnd;
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
