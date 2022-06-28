#include "boot_screen.h"

extern Screen mainScreen;
//extern UI* ui;

void BootScreen::onShow() {
    time = millis();
}

void BootScreen::loop() {
    if ((millis() - time) > ShowTime) {
        UI::getInstance()->showScreen(&mainScreen);
    }
}

void BootScreen::paint() {
    // TODO
}

