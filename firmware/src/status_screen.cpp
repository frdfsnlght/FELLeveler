#include "status_screen.h"
#include "display.h"

#include "network.h"
#include "bluetooth.h"
#include "accelerometer.h"

StatusScreen* StatusScreen::instance = nullptr;

StatusScreen* StatusScreen::getInstance() {
    if (instance == nullptr) instance = new StatusScreen();
    return instance;
}

StatusScreen::StatusScreen() : Screen() {
    // TODO: add listeners for connected impl, pitch, roll, angle changes
    Network::getInstance()->listeners.add(statusUpdated);
    Bluetooth::getInstance()->listeners.add(statusUpdated);
    Accelerometer::getInstance()->listeners.add(statusUpdated);
}

// TODO: add button handlers to change screens

void StatusScreen::paint() {
    Display* d = Display::getInstance();
    d->fillScreen(BLACK);
    d->setTextColor(WHITE);
    d->setTextSize(1);

    Network* n = Network::getInstance();
    d->printLeft("IP: ", 0, 0);
    d->print(n->ipAddress);
    d->printLeft("RSSI: ", 0, 10);
    d->print(n->rssi);
    d->print(" dBm");

    //Bluetooth* bt = Bluetooth::getInstance();
    d->printLeft("Bluetooth: ", 0, 20);

    Accelerometer* a = Accelerometer::getInstance();
    d->printLeft("Accel X: ", 0, 30);
    d->print(a->filtered.x);
    d->printLeft("Accel Y: ", 0, 40);
    d->print(a->filtered.y);
    d->printLeft("Accel Z: ", 0, 50);
    d->print(a->filtered.z);
}

void StatusScreen::statusUpdated(int ignore) {
    StatusScreen::getInstance()->dirty = true;
}
