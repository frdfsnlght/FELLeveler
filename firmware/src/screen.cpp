#include "screen.h"

#include "display.h"
#include "ui.h"

void Screen::show() {
    hidden = false;
    dirty = true;
    firstPaint = true;
    onShow();
}

void Screen::hide() {
    hidden = true;
    onHide();
}

bool Screen::isActiveScreen() {
    return UI::getInstance()->screen == this;
}

void Screen::paint() {
    if (! dirty) return;
    if (firstPaint || alwaysPaintBackground) {
        Display::getInstance()->setFont(-1);
        Display::getInstance()->setTextSize(1);
        paintBackground();
    }
    paintContent();
    dirty = firstPaint = false;
}

void Screen::paintBackground() {
    Display* d = Display::getInstance();
    d->fillScreen(backgroundColor);
}