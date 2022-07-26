#include "screens/implement_screen.h"

#include "ui.h"
#include "display.h"
#include "leveler.h"
#include "screens/tractor_screen.h"

ImplementScreen* ImplementScreen::instance = nullptr;

const char* ImplementScreen::ModeStrings[] = {
    "Tractor",
    "Earth"
};

ImplementScreen* ImplementScreen::getInstance() {
    if (instance == nullptr) instance = new ImplementScreen();
    return instance;
}

ImplementScreen::ImplementScreen() : Screen() {
    Leveler* leveler = Leveler::getInstance();
    leveler->remoteAnglesListeners.add([](void) { instance->dirty = true; });
    leveler->anglesListeners.add([](void) { instance->dirty = true; });
    alwaysPaintBackground = true;
    mode = Tractor;
}

void ImplementScreen::handleButtonRelease(Button* button) {
    if (mode == Tractor) {
        mode = Earth;
        dirty = true;
    } else if (mode == Earth) {
        mode = Tractor;
        UI::getInstance()->showScreen(TractorScreen::getInstance());
    }
}

void ImplementScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();

    int diff;
    if (mode == Tractor)
        diff = leveler->remotePitch - leveler->pitch;
    else if (mode == Earth)
        diff = leveler->remotePitch;

    char angle[10];
    sprintf(angle, "%.1f %c", (float)abs(diff) / 10.0f, (unsigned char)247);

    d->setTextSize(15);
    if (diff > 10) {
        d->setTextColor(RED);
        d->printCentered(">", d->width() / 2, 0);
        d->setTextSize(1);
        d->setTextColor(WHITE);
        d->printLeft(angle, 0, 50);
    } else if (diff < -10) {
        d->setTextColor(RED);
        d->printCentered("<", d->width() / 2, 0);
        d->setTextSize(1);
        d->setTextColor(WHITE);
        d->printRight(angle, d->width(), 50);
    } else {
        d->setTextColor(GREEN);
        d->printCentered("-", d->width() / 2, 0);
    }

    d->setTextColor(WHITE);
    d->setTextSize(1);
    d->printCentered(ModeStrings[mode], d->width() / 2, d->height() - 10);
}
