#include "screens/boot_screen.h"

#include <SPIFFS.h>

#include "ui.h"
#include "display.h"

BootScreen* BootScreen::instance = nullptr;

BootScreen* BootScreen::getInstance() {
    if (instance == nullptr) instance = new BootScreen();
    return instance;
}

void BootScreen::setup() {
    Display* d = Display::getInstance();
    d->loadImage("/boot.bmp", bootImage);
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
    d->drawImage(bootImage, (d->width() - bootImage.width()) / 2, (d->height() - bootImage.height())/ 2);
}

