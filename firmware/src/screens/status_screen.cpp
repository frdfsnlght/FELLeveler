#include "screens/status_screen.h"

#include "ui.h"
#include "display.h"
#include "network.h"
#include "netsock.h"
#include "leveler.h"
#include "config.h"

#include "screens/implement_screen.h"

StatusScreen* StatusScreen::instance = nullptr;

StatusScreen* StatusScreen::getInstance() {
    if (instance == nullptr) instance = new StatusScreen();
    return instance;
}

StatusScreen::StatusScreen() : Screen() {
    Network::getInstance()->stateChangedListeners.add([](void) {
        StatusScreen::getInstance()->networkChanged();
    });

    Netsock::getInstance()->stateChangedListeners.add([](void) {
        StatusScreen::getInstance()->netsockChanged();
    });
    Netsock::getInstance()->remoteDeviceChangedListeners.add([](void) {
        StatusScreen::getInstance()->netsockChanged();
    });

    Leveler::getInstance()->rollChangedListeners.add([](void) {
        StatusScreen::getInstance()->rollChanged();
    });
    Leveler::getInstance()->pitchChangedListeners.add([](void) {
        StatusScreen::getInstance()->pitchChanged();
    });
    Leveler::getInstance()->implementRollChangedListeners.add([](void) {
        if (Config::getInstance()->mode != Config::Tractor) return;
        StatusScreen::getInstance()->implementRollChanged();
    });
    Leveler::getInstance()->implementPitchChangedListeners.add([](void) {
        if (Config::getInstance()->mode != Config::Tractor) return;
        StatusScreen::getInstance()->implementPitchChanged();
    });

}

void StatusScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->showScreen(ImplementScreen::getInstance());
}

void StatusScreen::paintContent() {
    Display* d = Display::getInstance();
    d->setTextColor(WHITE);
    d->setTextSize(1);

    if (firstPaint || dirtyFlags.network) {
        d->printLeft("Wifi: ", 0, 0);
        d->fillRight(BLACK);
        Network* n = Network::getInstance();
        switch (n->state) {
            case Network::AP: d->print("AP"); break;
            case Network::Connect: d->print("Unconnected"); break;
            case Network::Connecting: d->print("Connecting"); break;
            case Network::Waiting: d->print("Waiting"); break;
            case Network::Connected: d->print(n->ipAddress); break;
        }
        dirtyFlags.network = false;
    }

    if (firstPaint || dirtyFlags.netsock) {
        d->printLeft("BT: ", 0, 10);
        d->fillRight(BLACK);
        Netsock* netsock = Netsock::getInstance();
        switch (netsock->state) {
            case Netsock::Idle: d->print("Idle"); break;
            case Netsock::Connected: {
                char tmpName[18]{0};
                strncpy(tmpName, netsock->remoteName, sizeof(tmpName) - 1);
                d->print(tmpName);
                break;
            }
            case Netsock::WaitingForClient: d->print("Listening"); break;
            case Netsock::ConnectToServer: d->print("Connecting"); break;
            case Netsock::WaitingToConnect: d->print("Waiting"); break;
        }
        dirtyFlags.netsock = false;
    }

    if (firstPaint || dirtyFlags.levelerRoll) {
        d->printLeft("Roll: ", 0, 20);
        d->fillRight(BLACK);
        Leveler* leveler = Leveler::getInstance();
        d->printf("%.1f ", (float)leveler->roll / 10.0);
        d->print((char)247);
        dirtyFlags.levelerRoll = false;
    }
    if (firstPaint || dirtyFlags.levelerPitch) {
        d->printLeft("Pitch: ", 0, 30);
        d->fillRight(BLACK);
        Leveler* leveler = Leveler::getInstance();
        d->printf("%.1f ", (float)leveler->pitch / 10.0);
        d->print((char)247);
        dirtyFlags.levelerPitch = false;
    }

    if (Config::getInstance()->mode != Config::Tractor) return;

    if (firstPaint || dirtyFlags.levelerImplementRoll) {
        d->printLeft("Roll: ", 0, 40);
        d->fillRight(BLACK);
        Leveler* leveler = Leveler::getInstance();
        d->printf("%.1f ", (float)leveler->implementRoll / 10.0);
        d->print((char)247);
        dirtyFlags.levelerImplementRoll = false;
    }
    if (firstPaint || dirtyFlags.levelerImplementPitch) {
        d->printLeft("Pitch: ", 0, 50);
        d->fillRight(BLACK);
        Leveler* leveler = Leveler::getInstance();
        d->printf("%.1f ", (float)leveler->implementPitch / 10.0);
        d->print((char)247);
        dirtyFlags.levelerImplementPitch = false;
    }

}

void StatusScreen::networkChanged() {
    dirtyFlags.network = dirty = true;
}

void StatusScreen::netsockChanged() {
    dirtyFlags.netsock = dirty = true;
}

void StatusScreen::rollChanged() {
    dirtyFlags.levelerRoll = dirty = true;
}

void StatusScreen::pitchChanged() {
    dirtyFlags.levelerPitch = dirty = true;
}

void StatusScreen::implementRollChanged() {
    dirtyFlags.levelerImplementRoll = dirty = true;
}

void StatusScreen::implementPitchChanged() {
    dirtyFlags.levelerImplementPitch = dirty = true;
}
