#include "screen.h"

#include "display.h"

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

void Screen::paint() {
    if (! dirty) return;
    if (firstPaint || alwaysPaintBackground) {
        Display::getInstance()->setFont(0);
        paintBackground();
    }
    paintContent();
    dirty = firstPaint = false;
}

void Screen::paintBackground() {
    Display* d = Display::getInstance();
    d->fillScreen(backgroundColor);
}