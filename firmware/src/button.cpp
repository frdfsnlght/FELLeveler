#include "button.h"

void Button::setup() {
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
    Serial.printf("Button setup complete on pin %d\n", pin);
}

void Button::loop() {
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
            //Serial.println("Button pressed");
            onPressListeners.call();
        } else {
            if (pressed) {
                if ((longPressTime > 0) && ((millis() - longPressedTime) >= longPressTime)) {
                    longPressedCount++;
                    longPressedTime = millis();
                    //Serial.print("Button long pressed ");
                    Serial.println(longPressedCount);
                    onLongPressListeners.call();
                }
            } else {
                if ((debounceTime > 0) && ((millis() - pressedTime) >= debounceTime)) {
                    pressedTime = millis();
                    longPressedTime = pressedTime;
                    pressed = true;
                    onPressListeners.call();
                    //Serial.println("Button pressed");
                }
            }
        }

    } else {
        if (pressed) {
            pressed = false;
            if (longPressedCount > 0) {
                onLongReleaseListeners.call();
                //Serial.println("Button long released");
            } else {
                onReleaseListeners.call();
                //Serial.println("Button released");
            }
        }
        lastState = LOW;
    }
}
