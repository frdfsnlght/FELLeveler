#include "status_screen.h"
#include "display.h"

StatusScreen::StatusScreen() : Screen() {
    // TODO: add listeners for wifi mode, wifi strength, wifi address, BT status, connected impl, etc
}

// TODO: add button handlers to change screens

void StatusScreen::paint() {
    Display* d = Display::getInstance();
    d->fillScreen(BLACK);
    d->setTextColor(WHITE);
    d->setTextSize(12);
    d->printLeft("WiFi: ", 0, 13);
    d->printLeft("Bluetooth: ", 0, 26);
}
