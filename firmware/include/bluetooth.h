#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
#include "callback_list.h"
#include "led.h"

class Bluetooth {

    public:

    static Bluetooth* getInstance();

    CallbackList<int> listeners = CallbackList<int>();

    void setup();
    void loop();
    
    private:

    static Bluetooth* instance;

    static const String BTBaseName;

    static void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);

    LED led = LED(2, false);
    BluetoothSerial bt = BluetoothSerial();
    bool master;
    bool connected;

    // Master stuff
    String implementName;
    //BTAddress implementAddress;

    Bluetooth() {}
    void handleCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    void receiveData();
    
};

#endif
