#include "screens/boot_screen.h"

#include "ui.h"
#include "display.h"

BootScreen* BootScreen::instance = nullptr;

BootScreen* BootScreen::getInstance() {
    if (instance == nullptr) instance = new BootScreen();
    return instance;
}

void BootScreen::onShow() {
    time = millis();
}

void BootScreen::loop() {
    if ((millis() - time) > ShowTime) {
        UI::getInstance()->nextScreen();
    }
}

void BootScreen::paintContent() {
    Display* d = Display::getInstance();
    d->drawBMP("/boot.bmp", 0,  36);
}

