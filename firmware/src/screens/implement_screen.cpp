#include "screens/implement_screen.h"

#include "config.h"
#include "ui.h"
#include "display.h"
#include "leveler.h"

#include "screens/tractor_screen.h"

ImplementScreen* ImplementScreen::instance = nullptr;

ImplementScreen* ImplementScreen::getInstance() {
    if (instance == nullptr) instance = new ImplementScreen();
    return instance;
}

void ImplementScreen::setup() {
    Leveler* leveler = Leveler::getInstance();
    leveler->remoteConnectedListeners.add([](void) {
        if ((Config::getInstance()->mode == Config::Tractor) && (! Leveler::getInstance()->remoteConnected)) {
            UI::getInstance()->showScreen(TractorScreen::getInstance());
        }
    });
    leveler->anglesListeners.add([](void) {
        instance->dirty = Config::getInstance()->mode == Config::Implement;
    });
    leveler->remoteAnglesListeners.add([](void) {
        instance->dirty = Config::getInstance()->mode == Config::Tractor;
    });
    alwaysPaintBackground = true;

    Display* d = Display::getInstance();
    d->loadImage("/implRollLeft1.bmp", rollLeft1Image);
    d->loadImage("/implRollLeft2.bmp", rollLeft2Image);
    d->loadImage("/implRollLevel.bmp", rollLevelImage);
    d->loadImage("/implRollRight1.bmp", rollRight1Image);
    d->loadImage("/implRollRight2.bmp", rollRight2Image);

    d->loadImage("/implPitchUp1.bmp", pitchUp1Image);
    d->loadImage("/implPitchUp2.bmp", pitchUp2Image);
    d->loadImage("/implPitchLevel.bmp", pitchLevelImage);
    d->loadImage("/implPitchDown1.bmp", pitchDown1Image);
    d->loadImage("/implPitchDown2.bmp", pitchDown2Image);
}

void ImplementScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->nextScreen();
}

void ImplementScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();
    float roll, pitch;
    char angle[10];

    switch (Config::getInstance()->mode) {
        case Config::Tractor:
            roll = (float)leveler->roll / 10.0;
            pitch = (float)leveler->pitch / 10.0;
            break;
        case Config::Implement:
            roll = (float)leveler->remoteRoll / 10.0;
            pitch = (float)leveler->remotePitch / 10.0;
            break;
    }

    d->setTextColor(WHITE);
    d->setFont(0);

    if (roll <= -10.0)
        d->drawImage(rollLeft2Image, 0, 0);
    else if (roll < -1.0)
        d->drawImage(rollLeft1Image, 0, 0);
    else if (roll <= 1.0)
        d->drawImage(rollLevelImage, 0, 0);
    else if (roll < 10.0)
        d->drawImage(rollRight1Image, 0, 0);
    else
        d->drawImage(rollRight2Image, 0, 0);

    sprintf(angle, "%.1f", roll);
    d->printRight(angle, d->width(), 32);

    if (pitch <= -10.0)
        d->drawImage(pitchDown2Image, 0, 0);
    else if (pitch < -1.0)
        d->drawImage(pitchDown1Image, 0, 0);
    else if (pitch <= 1.0)
        d->drawImage(pitchLevelImage, 0, 0);
    else if (pitch < 10.0)
        d->drawImage(pitchDown1Image, 0, 0);
    else
        d->drawImage(pitchDown2Image, 0, 0);

    sprintf(angle, "%.1f", pitch);
    d->printRight(angle, d->width(), 96);
}
