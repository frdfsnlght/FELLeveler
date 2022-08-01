#include "screens/leveler_screen.h"

#include "config.h"
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
        if (! instance->isActiveScreen()) return;
        instance->update();
        if (! Leveler::getInstance()->remoteConnected) {
            instance->mode = Tractor;
            UI::getInstance()->showScreen(TractorScreen::getInstance());
        }
    });
    leveler->remoteAnglesListeners.add([](void) {
        if (! instance->isActiveScreen()) return;
        instance->update();
    });
    leveler->anglesListeners.add([](void) {
        if (! instance->isActiveScreen()) return;
        instance->update();
    });
    alwaysPaintBackground = false;
    mode = Tractor;
    diff = INT_MAX;
    image = -1;
}

String LevelerScreen::saveState() {
    switch (mode) {
        case Tractor: return "Tractor";
        case Earth: return "Earth";
        default: return "";
    }
}

void LevelerScreen::restoreState(String& state) {
    if (state == "Tractor")
        mode = Tractor;
    else if (state == "Earth")
        mode = Earth;
    diff = INT_MAX;
    image = -1;
    update();
}

bool LevelerScreen::canShow() {
    return Config::getInstance()->running.mode == Config::Tractor;
}

void LevelerScreen::onShow() {
    update();
}

void LevelerScreen::handleButtonRelease(Button* button) {
    if (mode == Tractor) {
        mode = Earth;
        dirtyFlags.mode = true;
        dirty = true;
        UI::getInstance()->resetSave();
    } else if (mode == Earth) {
        mode = Tractor;
        dirtyFlags.mode = true;
        UI::getInstance()->nextScreen();
    }
}

void LevelerScreen::update() {
    Leveler* leveler = Leveler::getInstance();

    if (leveler->remoteConnected) {
        int newDiff;
        if (mode == Tractor)
            newDiff = leveler->remotePitch - leveler->pitch;
        else if (mode == Earth)
            newDiff = leveler->remotePitch;
        newDiff = newDiff / 10;  // get rid of fractional degrees
        if (newDiff != diff) {
            diff = newDiff;
            dirtyFlags.angle = true;
            int newImage;
            if (diff < -1) newImage = 2;        // left arrow
            else if (diff > 1) newImage = 3;    // right arrow
            else newImage = 1;                  // level
            if (newImage != image) {
                image = newImage;
                dirtyFlags.image = true;
            }
            dirty = true;
        }
    } else {
        if (image != 0) {
            image = 0;                          // none
            dirtyFlags.image = true;
            diff = INT_MAX;
            dirty = true;
        }
    }
}

void LevelerScreen::paintContent() {
    Display* d = Display::getInstance();

    d->setFont(0);
    d->setTextColor(WHITE);

    if (firstPaint || dirtyFlags.image) {
        int x0 = ((d->width() - BoxWidth) / 2) + (LineWidth / 2);
        int y0 = LineWidth / 2;
        int x1 = (x0 + BoxWidth) - LineWidth;
        int y1 = (y0 + BoxHeight) - LineWidth;
        d->fillRect((d->width() - BoxWidth) / 2, 0, BoxWidth, BoxHeight, BLACK);
        switch (image) {
            case 0:
                drawNone(d, x0, y0, x1, y1); break;
            case 1:
                drawLevel(d, x0, y0, x1, y1); break;
            case 2:
                drawLeft(d, x0, y0, x1, y1); break;
            case 3:
                drawRight(d, x0, y0, x1, y1); break;
        }
        dirtyFlags.image = false;
    }

    if (firstPaint || dirtyFlags.angle) {
        char angle[20];
        int x0 = (d->width() - BoxWidth) / 2;
        int x1 = x0 + BoxWidth;
        int y0 = BoxHeight / 2;
        switch (image) {
            case 2:
                sprintf(angle, "%d%c", abs(diff), (unsigned char)176);
                d->fillRect(x1 - 45, y0 - 5, 45, 13, BLACK);
                d->printRight(angle, x1 - 3, y0 + 7); // extra 3 pixels is because the degree symbol?
                break;
            case 3:
                sprintf(angle, "%d%c", abs(diff), (unsigned char)176);
                d->fillRect(x0, y0 - 5, 45, 13, BLACK);
                d->printLeft(angle, x0, y0 + 7);
                break;
        }
        dirtyFlags.angle = false;
    }

    if (firstPaint || dirtyFlags.mode) {
        d->fillRect(0, d->height() - 13, d->width(), 13, BLACK);
        d->printCentered(ModeStrings[mode], d->width() / 2, d->height() - 1);
        dirtyFlags.mode = false;
    }
}

void LevelerScreen::drawRight(Display* d, int x0, int y0, int x1, int y1) {
    d->drawThickLine(x0, y0, x1, y0 + ((y1 - y0) / 2), LineWidth, RED, true);
    d->drawThickLine(x0, y1, x1, y0 + ((y1 - y0) / 2), LineWidth, RED, true);
}

void LevelerScreen::drawLeft(Display* d, int x0, int y0, int x1, int y1) {
    d->drawThickLine(x1, y0, x0, y0 + ((y1- y0) / 2), LineWidth, RED, true);
    d->drawThickLine(x1, y1, x0, y0 + ((y1- y0) / 2), LineWidth, RED, true);
}

void LevelerScreen::drawLevel(Display* d, int x0, int y0, int x1, int y1) {
    d->drawThickLine(x0, y0 + ((y1 - y0) / 2), x1, y0 + ((y1 - y0) / 2), LineWidth, GREEN, true);
}

void LevelerScreen::drawNone(Display* d, int x0, int y0, int x1, int y1) {
    d->drawThickLine(x0, y0, x1, y1, LineWidth, RED, true);
    d->drawThickLine(x1, y0, x0, y1, LineWidth, RED, true);
}
