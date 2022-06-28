#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "callback_list.h"

enum ButtonPinBias {
    None,
    Pullup,
    Pulldown
};

class Button {

    public:

    CallbackList<Button*> onPressListeners = CallbackList<Button*>();
    CallbackList<Button*> onReleaseListeners = CallbackList<Button*>();
    CallbackList<Button*> onLongPressListeners = CallbackList<Button*>();
    CallbackList<Button*> onLongReleaseListeners = CallbackList<Button*>();

    Button(int pin, ButtonPinBias bias, bool invert, int debounceTime, int longPressTime) :
        pin(pin),
        bias(bias),
        invert(invert),
        debounceTime(debounceTime),
        longPressTime(longPressTime)
        {}

    void begin() {
        switch (bias) {
            case None:
                pinMode(pin, INPUT);
                break;
            case Pullup:
                pinMode(pin, INPUT_PULLUP);
                break;
            case Pulldown:
                pinMode(pin, INPUT_PULLDOWN);
                break;
        }
    }

    void update() {
        int state = digitalRead(pin);
        if (invert) state = !state;
        if (state == HIGH) {
            if (lastState == LOW) {
                pressedTime = millis();
                longPressedTime = pressedTime;
                longPressedCount = 0;
                lastState = HIGH;
                if (debounceTime > 0) return;
                pressed = true;
                onPressListeners.call(this);
            } else {
                if (pressed) {
                    if ((longPressTime > 0) && ((millis() - longPressedTime) >= longPressTime)) {
                        longPressedCount++;
                        longPressedTime = millis();
                        onLongPressListeners.call(this);
                    }
                } else {
                    if ((debounceTime > 0) && ((millis() - pressedTime) >= debounceTime)) {
                        pressedTime = millis();
                        longPressedTime = pressedTime;
                        pressed = true;
                        onPressListeners.call(this);
                    }
                }
            }

        } else {
            if (pressed) {
                pressed = false;
                if (longPressedCount > 0)
                    onLongReleaseListeners.call(this);
                else
                    onReleaseListeners.call(this);
            }
            lastState = LOW;
        }
    }

    private:

    int pin;
    ButtonPinBias bias;
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