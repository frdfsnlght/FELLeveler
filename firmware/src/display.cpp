#include "display.h"

Display* Display::instance = nullptr;

Display* Display::getInstance() {
    if (! instance) instance = new Display();
    return instance;
}

void Display::setup() {
    begin();
    // Rotate the display: 0=none, 1=90 CW, 2=180, 3=270 CW
    //setRotation(1);
    Serial.println("Display setup complete");
}

void Display::printLeft(const char* str, int x, int y) {
    setCursor(x, y);
    print(str);
}

void Display::printCentered(const char* str, int x, int y) {
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(str, x, y, &x1, &y1, &w, &h);
    setCursor(x - w / 2, y);
    print(str);
}

void Display::printRight(const char* str, int x, int y) {
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(str, x, y, &x1, &y1, &w, &h);
    setCursor(x - w, y);
    print(str);
}
