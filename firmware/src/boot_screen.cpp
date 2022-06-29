#include "boot_screen.h"
#include "display.h"

extern Screen mainScreen;
extern Screen statusScreen;

void BootScreen::onShow() {
    time = millis();
}

void BootScreen::loop() {
    if ((millis() - time) > ShowTime) {
        UI::getInstance()->showScreen(&statusScreen);
    }
}

void BootScreen::paint() {
    Display* d = Display::getInstance();
    d->fillScreen(BLACK);
    d->setTextColor(WHITE);
    d->setTextSize(13);
    d->printCentered("FELLeveler", d->Width / 2, d->Height / 2);
}

