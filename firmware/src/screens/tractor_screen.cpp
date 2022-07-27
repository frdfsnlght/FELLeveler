#include "screens/tractor_screen.h"

#include "ui.h"
#include "display.h"
#include "leveler.h"
#include "screens/status_screen.h"

TractorScreen* TractorScreen::instance = nullptr;

TractorScreen* TractorScreen::getInstance() {
    if (instance == nullptr) instance = new TractorScreen();
    return instance;
}

TractorScreen::TractorScreen() : Screen() {
    Leveler* leveler = Leveler::getInstance();
    leveler->anglesListeners.add([](void) { instance->dirty = true; });
}

void TractorScreen::handleButtonRelease(Button* button) {
    UI::getInstance()->showScreen(StatusScreen::getInstance());
}

void TractorScreen::paintContent() {
    Display* d = Display::getInstance();
    Leveler* leveler = Leveler::getInstance();

    d->setTextSize(1);
    d->setTextColor(WHITE);
    d->printLeft("Tractor stuff", 0, 10);
}
