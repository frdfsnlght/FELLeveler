#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "callback_list.h"

class Button {

    public:

    enum PinBias {
        None,
        Pullup,
        Pulldown
    };

    CallbackList onPressListeners = CallbackList();
    CallbackList onReleaseListeners = CallbackList();
    CallbackList onLongPressListeners = CallbackList();
    CallbackList onLongReleaseListeners = CallbackList();

    Button(int pin, PinBias bias, bool invert, int debounceTime, int longPressTime) :
        pin(pin),
        bias(bias),
        invert(invert),
        debounceTime(debounceTime),
        longPressTime(longPressTime)
        {}

    void setup();
    void loop();

    private:

    int pin;
    PinBias bias;
    bool invert;
    int debounceTime;
    int longPressTime;

    int lastState = LOW;
    bool pressed = false;
    unsigned long pressedTime;
    unsigned long longPressedTime;
    int longPressedCount;

};

#endif