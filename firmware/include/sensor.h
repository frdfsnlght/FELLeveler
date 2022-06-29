#ifndef SENSOR_H
#define SENSOR_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#include "filter_ewma.h"
#include "vector3.h"
#include "callback_list.h"

// ADXL345 uses right hand axis orientation
// GPIO 21 (SDA)
// GPIO 22 (SCL)

const unsigned long SENSOR_REPORT_INTERVAL = 100;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(1);
Vector3 accelRaw;
Vector3 accelFiltered;
CallbackList<Vector3*> accelRawListeners = CallbackList<Vector3*>();
CallbackList<Vector3*> accelFilteredListeners = CallbackList<Vector3*>();
FilterEWMA accelX, accelY, accelZ;
bool _sensorSetup = false;
unsigned long sensorLastReportTime = 0;


float getSensorDataRate() {
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

int getSensorRange() {
    switch (accel.getRange()) {
        case ADXL345_RANGE_16_G: return 16;
        case ADXL345_RANGE_8_G: return 8;
        case ADXL345_RANGE_4_G: return 4;
        case ADXL345_RANGE_2_G: return 2;
        default: return 0;
    }
}

void displaySensorDetails() {
    if (! _sensorSetup) return;
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

void setupSensor() {
    if (! accel.begin()) {
        Serial.println("ADXL343 not detected!");
        return;
    }
    accel.setRange(ADXL345_RANGE_2_G);
    _sensorSetup = true;
    displaySensorDetails();
}

void loopSensor() {
    sensors_event_t event;
    accel.getEvent(&event);
    accelRaw.x = event.acceleration.x;
    accelRaw.y = event.acceleration.y;
    accelRaw.z = event.acceleration.z;
    accelX.filter(accelRaw.x);
    accelY.filter(accelRaw.y);
    accelZ.filter(accelRaw.z);
    accelRawListeners.call(&accelRaw);

    if ((millis() - sensorLastReportTime) > SENSOR_REPORT_INTERVAL) {
        sensorLastReportTime = millis();

        accelFiltered.x = accelX.value;
        accelFiltered.y = accelY.value;
        accelFiltered.z = accelZ.value;

        Serial.print("ACCEL: ");
        Serial.print(accelRaw.x); Serial.print(',');
        Serial.print(accelRaw.y); Serial.print(',');
        Serial.print(accelRaw.z); Serial.print(',');
        Serial.print(accelFiltered.x); Serial.print(',');
        Serial.print(accelFiltered.y); Serial.print(',');
        Serial.println(accelFiltered.z);

        accelFilteredListeners.call(&accelFiltered);
    }
}

#endif
