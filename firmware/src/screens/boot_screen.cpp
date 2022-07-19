#include "screens/boot_screen.h"

#include "ui.h"
#include "display.h"

#include "screens/implement_screen.h"

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
        UI::getInstance()->showScreen(ImplementScreen::getInstance());
    }
}

void BootScreen::paintContent() {
    Display* d = Display::getInstance();
    d->setTextColor(WHITE);
    d->setTextSize(2);
    d->printCentered("FELLeveler", d->width() / 2, d->height() / 2);
}

