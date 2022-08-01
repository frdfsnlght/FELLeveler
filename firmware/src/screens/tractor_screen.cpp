#include "screens/tractor_screen.h"

#include "config.h"
#include "ui.h"
#include "display.h"
#include "leveler.h"

#include "screens/leveler_screen.h"

TractorScreen* TractorScreen::instance = nullptr;
const char* TractorScreen::Images[] = {
    "/tracRollLeft1.bmp",
    "/tracRollLeft2.bmp",
    "/tracRollLevel.bmp",
    "/tracRollRight1.bmp",
    "/tracRollRight2.bmp",
    "/tracPitchDown1.bmp",
    "/tracPitchDown2.bmp",
    "/tracPitchLevel.bmp",
    "/tracPitchUp1.bmp",
    "/tracPitchUp2.bmp"
};

TractorScreen* TractorScreen::getInstance() {
    if (instance == nullptr) instance = new TractorScreen();
    return instance;
}

void TractorScreen::setup() {
    Leveler* leveler = Leveler::getInstance();
    leveler->remoteConnectedListeners.add([](void) {
        if (! instance->isActiveScreen()) return;
        if (Leveler::getInstance()->remoteConnected) {
            UI::getInstance()->showScreen(LevelerScreen::getInstance());
        }
    });
    leveler->anglesListeners.add([](void) { instance->update(); });
    roll = pitch = INT_MAX;
    rollImage = pitchImage = -1;
}

bool TractorScreen::canShow() {
    return Config::getInstance()->running.mode == Config::Tractor;
}

void TractorScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->nextScreen();
}

void TractorScreen::update() {
    Leveler* leveler = Leveler::getInstance();

    if (leveler->roll != roll) {
        roll = leveler->roll;
        dirtyFlags.roll = true;
        int newImage;
        if (roll <= -100) newImage = 1;
        else if (roll < -10) newImage = 0;
        else if (roll <= 10) newImage = 2;
        else if (roll < 100) newImage = 3;
        else newImage = 4;
        if (rollImage != newImage) {
            rollImage = newImage;
            dirtyFlags.rollImage = true;
        }
        dirty = true;
    }
    if (leveler->pitch != pitch) {
        pitch = leveler->pitch;
        dirtyFlags.pitch = true;
        int newImage;
        if (pitch <= -100) newImage = 6;
        else if (pitch < -10) newImage = 5;
        else if (pitch <= 10) newImage = 7;
        else if (pitch < 100) newImage = 8;
        else newImage = 9;
        if (pitchImage != newImage) {
            pitchImage = newImage;
            dirtyFlags.pitchImage = true;
        }
        dirty = true;
    }

}

void TractorScreen::onShow() {
    update();
}

void TractorScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();
    float angleF;
    char angle[10];

    d->setTextColor(WHITE);
    d->setFont(0);

    if (firstPaint || dirtyFlags.rollImage) {
        d->drawImage(Images[rollImage], 0, 0);
        dirtyFlags.rollImage = false;
    }
    if (firstPaint || dirtyFlags.roll) {
        angleF = (float)roll / 10.0;
        d->fillRect(72, 18, 64, 31, BLACK);
        sprintf(angle, "%.1f%c", angleF, (unsigned char)176);
        d->printLeft(angle, 72, 30);
        sprintf(angle, "%.0f%%", abs(tan(DEG_TO_RAD * angleF)) * 100.0);
        d->printLeft(angle, 72, 48);
        dirtyFlags.roll = false;
    }

    if (firstPaint || dirtyFlags.pitchImage) {
        d->drawImage(Images[pitchImage], 0, 64);
        dirtyFlags.pitchImage = false;
    }
    if (firstPaint || dirtyFlags.pitch) {
        angleF = (float)pitch / 10.0;
        sprintf(angle, "%.1f%c", angleF, (unsigned char)176);
        d->fillRect(72, 82, 64, 31, BLACK);
        d->printLeft(angle, 72, 94);
        sprintf(angle, "%.0f%%", abs(tan(DEG_TO_RAD * angleF)) * 100.0);
        d->printLeft(angle, 72, 112);
        dirtyFlags.pitch = false;
    }
}
