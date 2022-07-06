#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "callback_list.h"

class Bluetooth {

    public:

    static Bluetooth* getInstance();

    CallbackList<int> listeners = CallbackList<int>();

    void setup();
    void loop();
    
    private:

    static Bluetooth* instance;

    Bluetooth() {}

};

#endif
