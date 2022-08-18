#include "screens/implement_screen.h"

#include "config.h"
#include "ui.h"
#include "display.h"
#include "leveler.h"

#include "screens/tractor_screen.h"

ImplementScreen* ImplementScreen::instance = nullptr;
const char* ImplementScreen::ImageFiles[] = {
    "/implRollLeft1.img",
    "/implRollLeft2.img",
    "/implRollLevel.img",
    "/implRollRight1.img",
    "/implRollRight2.img",
    "/implPitchDown1.img",
    "/implPitchDown2.img",
    "/implPitchLevel.img",
    "/implPitchUp1.img",
    "/implPitchUp2.img"
};
SPIFFS_Img ImplementScreen::Images[MaxImages];
const uint16_t ImplementScreen::Colors[] = {
    YELLOW,
    RED,
    GREEN,
    YELLOW,
    RED,
    YELLOW,
    RED,
    GREEN,
    YELLOW,
    RED
};

ImplementScreen* ImplementScreen::getInstance() {
    if (instance == nullptr) instance = new ImplementScreen();
    return instance;
}

void ImplementScreen::setup() {
    Leveler* leveler = Leveler::getInstance();
    leveler->remoteConnectedListeners.add([](void) {
        if (! instance->isActiveScreen()) return;
        if ((Config::getInstance()->running.mode == Config::Tractor) && (! Leveler::getInstance()->remoteConnected)) {
            UI::getInstance()->showScreen(TractorScreen::getInstance());
        }
    });
    leveler->anglesListeners.add([](void) { instance->update(); });
    leveler->remoteAnglesListeners.add([](void) { instance->update(); });
    roll = pitch = INT_MAX;
    rollImage = pitchImage = -1;
    
    for (int i = 0; i < MaxImages; i++)
        Display::getInstance()->loadImg(ImageFiles[i], Images[i]);
}


void ImplementScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->nextScreen();
}

void ImplementScreen::update() {
    Config* config = Config::getInstance();
    Leveler* leveler = Leveler::getInstance();
    int inRoll, inPitch;

    if (config->running.mode == Config::Tractor) {
        inRoll = leveler->remoteRoll;
        inPitch = leveler->remotePitch;
    } else if (config->running.mode == Config::Implement) {
        inRoll = leveler->roll;
        inPitch = leveler->pitch;
    }

    if (inRoll != roll) {
        roll = inRoll;
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
    if (inPitch != pitch) {
        pitch = inPitch;
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

void ImplementScreen::onShow() {
    update();
}

void ImplementScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();
    float angleF;
    char angle[10];

    d->setTextColor(WHITE);
    d->setFont(0);

    if (firstPaint || dirtyFlags.rollImage) {
        d->drawImg(Images[rollImage], 0, 0, Colors[rollImage]);
        dirtyFlags.rollImage = false;
    }
    if (firstPaint || dirtyFlags.roll) {
        angleF = (float)roll / 10.0;
        d->fillRect(72, 26, 64, 13, BLACK);
        sprintf(angle, "%.1f%c", angleF, (unsigned char)176);
        d->printLeft(angle, 72, 38);
        dirtyFlags.roll = false;
    }

    if (firstPaint || dirtyFlags.pitchImage) {
        d->drawImg(Images[pitchImage], 0, 64, Colors[pitchImage]);
        dirtyFlags.pitchImage = false;
    }
    if (firstPaint || dirtyFlags.pitch) {
        angleF = (float)pitch / 10.0;
        sprintf(angle, "%.1f%c", angleF, (unsigned char)176);
        d->fillRect(72, 90, 64, 13, BLACK);
        d->printLeft(angle, 72, 102);
        dirtyFlags.pitch = false;
    }
}
