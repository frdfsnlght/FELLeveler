#include "status_screen.h"
#include "display.h"

//#include "network.h"

StatusScreen* StatusScreen::instance = nullptr;

StatusScreen* StatusScreen::getInstance() {
    if (instance == nullptr) instance = new StatusScreen();
    return instance;
}

StatusScreen::StatusScreen() : Screen() {
    // TODO: add listeners for BT status, connected impl, pitch, roll, angle changes
    Network::getInstance()->networkListeners.add(networkUpdated);
    //Bluetooth::getInstance()->bluetoothListeners.add(bluetoothUpdated);
    Accelerometer::getInstance()->accelerometerListeners.add(accelerometerUpdated);
}

// TODO: add button handlers to change screens

void StatusScreen::paint() {
    Display* d = Display::getInstance();
    d->fillScreen(BLACK);
    d->setTextColor(WHITE);
    d->setTextSize(12);
    d->printLeft("WiFi: ", 0, 13);
    d->printLeft("Bluetooth: ", 0, 26);
    d->printLeft("Accel: ", 0, 39);
}

void StatusScreen::networkUpdated(Network* network) {
    StatusScreen::getInstance()->dirty = true;
}

//void StatusScreen::bluetoothUpdated(Bluetooth* bt) {
//    StatusScreen::getInstance()->dirty = true;
//}

void StatusScreen::accelerometerUpdated(Accelerometer* accel) {
    StatusScreen::getInstance()->dirty = true;
}
