#ifndef LED_H
#define LED_H

#include <Arduino.h>

class LED {

    public:

    LED(int pin, bool invert) :
        pin(pin),
        invert(invert)
        {}

    void turnOn() {
        blinking = false;
        on = true;
        digitalWrite(pin, invert ? LOW : HIGH);
    }

    void turnOff() {
        blinking = false;
        on = false;
        digitalWrite(pin, invert ? HIGH : LOW);
    }

    void blink(uint onTime = 500, uint offTime = 500, bool startOn = true) {
        blinkOnTime = onTime;
        blinkOffTime = offTime;
        blinking = true;
        lastTime = millis();
        on = startOn;
        if (on)
            digitalWrite(pin, invert ? LOW : HIGH);
        else
            digitalWrite(pin, invert ? HIGH : LOW);
    }

    void setup() {
        pinMode(pin, OUTPUT);
    }

    void loop() {
        if (! blinking) return;
        if (on) {
            if ((millis() - lastTime) >= blinkOnTime) {
                on = false;
                lastTime = millis();
                digitalWrite(pin, invert ? HIGH : LOW);
            }
        } else {
            if ((millis() - lastTime) >= blinkOffTime) {
                on = true;
                lastTime = millis();
                digitalWrite(pin, invert ? LOW : HIGH);
            }
        }
    }

    private:

    int pin;
    bool invert;
    bool blinking = false;
    uint blinkOnTime;
    uint blinkOffTime;

    bool on = false;
    unsigned long lastTime;

};

#endif