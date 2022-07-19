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
    leveler->implementPitchChangedListeners.add([](void) { instance->dirty = true; });
    leveler->pitchChangedListeners.add([](void) { instance->dirty = true; });
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

    d->setTextSize(18);
    int diff;
    if (mode == Tractor)
        diff = leveler->implementPitch - leveler->pitch;
    else if (mode == Earth)
        diff = leveler->implementPitch;

    char angle[10];
    sprintf(angle, "%.1f %c", abs(diff), (char)247);

    if (diff > 10) {
        d->setTextColor(RED);
        d->printCentered(">", d->width() / 2, 0);
        d->setTextSize(1);
        d->setTextColor(WHITE);
        d->printLeft(angle, 0, 60);
    } else if (diff < 10) {
        d->setTextColor(RED);
        d->printCentered("<", d->width() / 2, 0);
        d->setTextSize(1);
        d->setTextColor(WHITE);
        d->printRight(angle, d->width(), 60);
    } else {
        d->setTextColor(GREEN);
        d->printCentered("-", d->width() / 2, 0);
    }

    d->setTextSize(1);
    d->printCentered(ModeStrings[mode], d->width() / 2, d->height() - 10);
}
