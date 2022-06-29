#ifndef STATUS_SCREEN_H
#define STATUS_SCREEN_H

#include "screen.h"
#include "network.h"
#include "accelerometer.h"

class StatusScreen : public Screen {

    public:

    static StatusScreen* getInstance();

    void paint();

    private:

    static StatusScreen* instance;

    static void networkUpdated(Network* network);
    //static void bluetoothUpdated(Bluetooth* bt);
    static void accelerometerUpdated(Accelerometer* accel);

    StatusScreen();


};

#endif
