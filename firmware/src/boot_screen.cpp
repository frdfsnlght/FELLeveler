#include "boot_screen.h"
#include "display.h"

#include "status_screen.h"

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
        UI::getInstance()->showScreen(StatusScreen::getInstance());
    }
}

void BootScreen::paint() {
    Display* d = Display::getInstance();
    d->fillScreen(BLACK);
    d->setTextColor(WHITE);
    d->setTextSize(13);
    d->printCentered("FELLeveler", d->Width / 2, d->Height / 2);
}

