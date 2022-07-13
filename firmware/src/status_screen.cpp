#include "status_screen.h"
#include "display.h"

#include "network.h"
#include "bluetooth.h"
#include "leveler.h"

StatusScreen* StatusScreen::instance = nullptr;

StatusScreen* StatusScreen::getInstance() {
    if (instance == nullptr) instance = new StatusScreen();
    return instance;
}

StatusScreen::StatusScreen() : Screen() {
    Network::getInstance()->stateChangedListeners.add([](void) {
        StatusScreen::getInstance()->networkStateChanged();
    });
    Network::getInstance()->wifiModeChangedListeners.add([](void) {
        StatusScreen::getInstance()->networkStateChanged();
    });
    Network::getInstance()->wifiRSSIChangedListeners.add([](void) {
        StatusScreen::getInstance()->networkRSSIChanged();
    });

    Bluetooth::getInstance()->connectedChangedListeners.add([](void) {
        StatusScreen::getInstance()->btChanged();
    });
    Bluetooth::getInstance()->connectedDeviceChangedListeners.add([](void) {
        StatusScreen::getInstance()->btChanged();
    });

    Leveler::getInstance()->rollChangedListeners.add([](void) {
        StatusScreen::getInstance()->rollChanged();
    });
    Leveler::getInstance()->pitchChangedListeners.add([](void) {
        StatusScreen::getInstance()->pitchChanged();
    });
    Leveler::getInstance()->implementRollChangedListeners.add([](void) {
        StatusScreen::getInstance()->implementRollChanged();
    });
    Leveler::getInstance()->implementPitchChangedListeners.add([](void) {
        StatusScreen::getInstance()->implementPitchChanged();
    });

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

    //Leveler* leveler = Leveler::getInstance();
    d->printLeft("Leveler: ", 0, 30);

}

void StatusScreen::networkStateChanged() {
    // TODO
    dirty = true;
}

void StatusScreen::networkRSSIChanged() {
    // TODO
    dirty = true;
}

void StatusScreen::btChanged() {
    // TODO
    dirty = true;
}

void StatusScreen::rollChanged() {
    // TODO
    dirty = true;
}

void StatusScreen::pitchChanged() {
    // TODO
    dirty = true;
}

void StatusScreen::implementRollChanged() {
    // TODO
    dirty = true;
}

void StatusScreen::implementPitchChanged() {
    // TODO
    dirty = true;
}
