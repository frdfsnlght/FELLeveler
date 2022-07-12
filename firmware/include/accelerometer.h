#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Adafruit_ADXL345_U.h>

#include "vector3.h"
#include "callback_list.h"
#include "filter_ewma.h"

// ADXL345 uses right hand axis orientation

// I2C port
// SDA - 21
// SCL - 22

class Accelerometer {

    public:

    static Accelerometer* getInstance();

    Vector3 raw;
    Vector3 filtered;
    CallbackList listeners = CallbackList();

    void setup();
    void loop();

    private:

    static Accelerometer* instance;
    static const unsigned long UpdateInterval = 100;

    Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
    FilterEWMA filterX, filterY, filterZ;
    bool setupComplete = false;
    unsigned long lastUpdate = 0;

    Accelerometer() {}

    float getSensorDataRate();
    int getSensorRange();
    void displaySensorDetails();

};

#endif
