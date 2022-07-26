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

void Display::fillRight(uint16_t color) {
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds("X", cursor_x, cursor_y, &x1, &y1, &w, &h);
    fillRect(cursor_x, cursor_y, _width - cursor_x, h, color);
}

void Display::drawThickLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color, bool rounded) {
    float dx = (width / 2.0) * (x0 - x1) / sqrt(sq(x0 - x1) + sq(y0 - y1));
    float dy = (width / 2.0) * (y0 - y1) / sqrt(sq(x0 - x1) + sq(y0 - y1));
    fillTriangle(x0 + dx, y0 - dy, x0 - dx, y0 + dy, x1 + dx, y1 - dy, color);
    fillTriangle(x0 - dx, y0 + dy, x1 - dx, y1 + dy, x1 + dx, y1 - dy, color);
    if (rounded) {
        fillCircle(x0, y0, (uint16_t)(width / 2.0), color);
        fillCircle(x1, y1, (uint16_t)(width / 2.0), color);
    }
}

