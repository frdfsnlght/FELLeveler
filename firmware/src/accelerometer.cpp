#include "accelerometer.h"

//#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include "filter_ewma.h"
#include "vector3.h"
#include "callback_list.h"

Accelerometer* Accelerometer::instance = nullptr;

Accelerometer* Accelerometer::getInstance() {
    if (instance == nullptr) instance = new Accelerometer();
    return instance;
}

float Accelerometer::getSensorDataRate() {
    switch (accel.getDataRate()) {
        case ADXL345_DATARATE_3200_HZ: return 3200;
        case ADXL345_DATARATE_1600_HZ: return 1600;
        case ADXL345_DATARATE_800_HZ: return 800;
        case ADXL345_DATARATE_400_HZ: return 400;
        case ADXL345_DATARATE_200_HZ: return 200;
        case ADXL345_DATARATE_100_HZ: return 100;
        case ADXL345_DATARATE_50_HZ: return 50;
        case ADXL345_DATARATE_25_HZ: return 25;
        case ADXL345_DATARATE_12_5_HZ: return 12.5;
        case ADXL345_DATARATE_6_25HZ: return 6.25;
        case ADXL345_DATARATE_3_13_HZ: return 3.13;
        case ADXL345_DATARATE_1_56_HZ: return 1.56;
        case ADXL345_DATARATE_0_78_HZ: return 0.78;
        case ADXL345_DATARATE_0_39_HZ: return 0.39;
        case ADXL345_DATARATE_0_20_HZ: return 0.20;
        case ADXL345_DATARATE_0_10_HZ: return 0.1;
        default: return 0;
    }
}

int Accelerometer::getSensorRange() {
    switch (accel.getRange()) {
        case ADXL345_RANGE_16_G: return 16;
        case ADXL345_RANGE_8_G: return 8;
        case ADXL345_RANGE_4_G: return 4;
        case ADXL345_RANGE_2_G: return 2;
        default: return 0;
    }
}

void Accelerometer::displaySensorDetails() {
    if (! setupComplete) return;
    sensor_t sensor;
    accel.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print("Sensor:       "); Serial.println(sensor.name);
    Serial.print("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" m/s^2");
    Serial.print("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" m/s^2");
    Serial.print("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" m/s^2");
    Serial.print("Data Rate:    "); Serial.print(getSensorDataRate()); Serial.println(" Hz");
    Serial.print("Range:        "); Serial.print(getSensorRange()); Serial.println(" g");
    Serial.println("------------------------------------");
}

void Accelerometer::setup() {
    if (! accel.begin()) {
        Serial.println("ADXL343 not detected!");
        return;
    }
    accel.setRange(ADXL345_RANGE_2_G);
    setupComplete = true;
    displaySensorDetails();
}

void Accelerometer::loop() {
    if (! setupComplete) return;
    
    if ((millis() - lastUpdate) < UpdateInterval) return;
    lastUpdate = millis();

    sensors_event_t event;
    accel.getEvent(&event);
    raw.x = event.acceleration.x;
    raw.y = event.acceleration.y;
    raw.z = event.acceleration.z;
    filterX.filter(raw.x);
    filterY.filter(raw.y);
    filterZ.filter(raw.z);

    bool changed = false;
    if (filtered.x != filterX.value) {
        filtered.x = filterX.value;
        changed = true;
    }
    if (filtered.y != filterY.value) {
        filtered.y = filterY.value;
        changed = true;
    }
    if (filtered.z != filterZ.value) {
        filtered.z = filterZ.value;
        changed = true;
    }

    if (changed)
        listeners.call();

    log_v("ACCEL: %.3f, %.3f, %.3f, %.3f, %.3f, %.3f", raw.x, raw.y, raw.z, filtered.x, filtered.y, filtered.z);
    
}


