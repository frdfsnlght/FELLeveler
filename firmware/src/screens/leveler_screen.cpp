#include "screens/leveler_screen.h"

#include "ui.h"
#include "display.h"
#include "leveler.h"

#include "screens/tractor_screen.h"

LevelerScreen* LevelerScreen::instance = nullptr;

const char* LevelerScreen::ModeStrings[] = {
    "Tractor",
    "Earth"
};

LevelerScreen* LevelerScreen::getInstance() {
    if (instance == nullptr) instance = new LevelerScreen();
    return instance;
}

void LevelerScreen::setup() {
    Leveler* leveler = Leveler::getInstance();
    leveler->remoteConnectedListeners.add([](void) {
        if (! Leveler::getInstance()->remoteConnected) {
            instance->mode = Tractor;
            UI::getInstance()->showScreen(TractorScreen::getInstance());
        }
    });
    leveler->remoteAnglesListeners.add([](void) { instance->dirty = true; });
    leveler->anglesListeners.add([](void) { instance->dirty = true; });
    alwaysPaintBackground = true;
    mode = Tractor;

    Display* d = Display::getInstance();
    d->loadImage("/levLeft.bmp", leftImage);
    d->loadImage("/levRight.bmp", rightImage);
    d->loadImage("/levLevel.bmp", levelImage);
    d->loadImage("/levNone.bmp", noneImage);
}

void LevelerScreen::handleButtonRelease(Button* button) {
    if (mode == Tractor) {
        mode = Earth;
        dirty = true;
    } else if (mode == Earth) {
        mode = Tractor;
        UI::getInstance()->nextScreen();
    }
}

void LevelerScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();

    d->setTextColor(WHITE);
    d->printCentered(ModeStrings[mode], d->width() / 2, d->height() - 10);

    if (leveler->remoteConnected) {
        int diff;
        if (mode == Tractor)
            diff = leveler->remotePitch - leveler->pitch;
        else if (mode == Earth)
            diff = leveler->remotePitch;

        char angle[10];
        sprintf(angle, "%.1f %c", (float)abs(diff) / 10.0f, (unsigned char)247);

        d->setTextColor(WHITE);

        if (diff > 10) {
            d->drawImage(rightImage, (d->width() - rightImage.width()) / 2, 0);
            d->printLeft(angle, 0, 50);
        } else if (diff < -10) {
            d->drawImage(leftImage, (d->width() - leftImage.width()) / 2, 0);
            d->printRight(angle, d->width(), 50);
        } else {
            d->drawImage(levelImage, (d->width() - leftImage.width()) / 2, 0);
        }
    } else {
        d->drawImage(noneImage, (d->width() - leftImage.width()) / 2, 0);
    }

}
