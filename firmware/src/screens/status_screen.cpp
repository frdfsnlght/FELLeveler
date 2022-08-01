#include "screens/status_screen.h"

#include "ui.h"
#include "display.h"
#include "network.h"
#include "leveler.h"
#include "config.h"

#include "screens/implement_screen.h"

StatusScreen* StatusScreen::instance = nullptr;

StatusScreen* StatusScreen::getInstance() {
    if (instance == nullptr) instance = new StatusScreen();
    return instance;
}

StatusScreen::StatusScreen() : Screen() {
    Network::getInstance()->stateListeners.add([](void) {
        instance->dirty = instance->dirtyFlags.network = true;
    });

    Leveler::getInstance()->anglesListeners.add([](void) {
        instance->dirty = instance->dirtyFlags.angles = true;
    });
    Leveler::getInstance()->remoteConnectedListeners.add([](void) {
        instance->dirty = instance->dirtyFlags.remote = true;
    });
    Leveler::getInstance()->remoteInfoListeners.add([](void) {
        instance->dirty = instance->dirtyFlags.remote = true;
    });
    Leveler::getInstance()->remoteAnglesListeners.add([](void) {
        if (Config::getInstance()->running.mode != Config::Tractor) return;
        instance->dirty = instance->dirtyFlags.remoteAngles = true;
    });

}

void StatusScreen::loop() {
    static unsigned long lastMemoryCheck = 0;
    if ((millis() - lastMemoryCheck) > 1000) {
        lastMemoryCheck = millis();
        dirty = dirtyFlags.memory = true;
    }
}

void StatusScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->nextScreen();
}

void StatusScreen::paintContent() {
    Display* d = Display::getInstance();
    d->setFont(-1);
    d->setTextColor(WHITE);
    d->setTextSize(1);

    if (firstPaint || dirtyFlags.network) {
        d->printLeft("Wifi: ", 0, 0);
        d->fillRight(BLACK);
        Network* n = Network::getInstance();
        switch (n->state) {
            case Network::OTA: d->print("Updating"); break;
            case Network::Disconnect: d->print("Unconnected"); break;
            case Network::Connect: d->print("Unconnected"); break;
            case Network::Connecting: d->print("Connecting"); break;
            case Network::Connected: d->print(n->ipAddress); break;
            default:
                d->print(Network::StateStrings[n->state]);
                break;
        }
        dirtyFlags.network = false;
    }

    if (firstPaint || dirtyFlags.memory) {
        uint32_t free = ESP.getMaxAllocHeap();
        d->printLeft("Mem: ", 0, 10);
        d->fillRight(BLACK);
        d->print(free);
        dirtyFlags.memory = false;
    }

    if (firstPaint || dirtyFlags.angles) {
        Leveler* leveler = Leveler::getInstance();
        d->printLeft("Roll: ", 0, 20);
        d->fillRight(BLACK);
        d->printf("%.1f ", (float)leveler->roll / 10.0);
        d->print((char)247);
        d->printLeft("Pitch: ", 0, 30);
        d->fillRight(BLACK);
        d->printf("%.1f ", (float)leveler->pitch / 10.0);
        d->print((char)247);
        dirtyFlags.angles = false;
    }

    if (firstPaint || dirtyFlags.remote) {
        Leveler* leveler = Leveler::getInstance();
        d->printLeft("Remote: ", 0, 40);
        d->fillRight(BLACK);
        d->print(leveler->remoteConnected ? leveler->remoteName : "None");
        dirtyFlags.remote = false;
    }

    if (Config::getInstance()->running.mode != Config::Tractor) return;

    if (firstPaint || dirtyFlags.remoteAngles) {
        Leveler* leveler = Leveler::getInstance();
        d->printLeft("Impl Roll: ", 0, 50);
        d->fillRight(BLACK);
        d->printf("%.1f ", (float)leveler->remoteRoll / 10.0);
        d->print((char)247);
        d->printLeft("Impl Pitch: ", 0, 60);
        d->fillRight(BLACK);
        d->printf("%.1f ", (float)leveler->remotePitch / 10.0);
        d->print((char)247);
    }

}
