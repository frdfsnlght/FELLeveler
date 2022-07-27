#include "screens/tractor_screen.h"

#include "ui.h"
#include "display.h"
#include "leveler.h"

#include "screens/leveler_screen.h"

TractorScreen* TractorScreen::instance = nullptr;

TractorScreen* TractorScreen::getInstance() {
    if (instance == nullptr) instance = new TractorScreen();
    return instance;
}

void TractorScreen::setup() {
    Leveler* leveler = Leveler::getInstance();
    leveler->remoteConnectedListeners.add([](void) {
        if (Leveler::getInstance()->remoteConnected) {
            UI::getInstance()->showScreen(LevelerScreen::getInstance());
        }
    });
    leveler->anglesListeners.add([](void) { instance->dirty = true; });
    alwaysPaintBackground = true;

    Display* d = Display::getInstance();
    d->loadImage("/tracRollLeft1.bmp", rollLeft1Image);
    d->loadImage("/tracRollLeft2.bmp", rollLeft2Image);
    d->loadImage("/tracRollLevel.bmp", rollLevelImage);
    d->loadImage("/tracRollRight1.bmp", rollRight1Image);
    d->loadImage("/tracRollRight2.bmp", rollRight2Image);

    d->loadImage("/tracPitchUp1.bmp", pitchUp1Image);
    d->loadImage("/tracPitchUp2.bmp", pitchUp2Image);
    d->loadImage("/tracPitchLevel.bmp", pitchLevelImage);
    d->loadImage("/tracPitchDown1.bmp", pitchDown1Image);
    d->loadImage("/tracPitchDown2.bmp", pitchDown2Image);
}

void TractorScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->nextScreen();
}

void TractorScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();
    float roll, pitch;
    char angle[10];

    roll = (float)leveler->roll / 10.0;
    pitch = (float)leveler->pitch / 10.0;

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
    d->printRight(angle, d->width(), 22);
    sprintf(angle, "%.0f%%", tan(DEG_TO_RAD * roll) * 100.0);
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
    d->printRight(angle, d->width(), 86);
    sprintf(angle, "%.0f%%", tan(DEG_TO_RAD * pitch) * 100.0);
    d->printRight(angle, d->width(), 92);
}
