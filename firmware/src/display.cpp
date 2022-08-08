#include "display.h"

#include "config.h"
#include "FreeSans9pt8b.h"

Display* Display::instance = nullptr;

const GFXfont* Display::Fonts[] = {
    &FreeSans9pt8b
};

Display* Display::getInstance() {
    if (! instance) instance = new Display();
    return instance;
}

void Display::setup() {
    begin();
    // Rotate the display: 0=none, 1=90 CW, 2=180, 3=270 CW
    setRotation(2);
    Serial.println("Display setup complete");
    setFont(-1);
}

bool Display::loadImage(const char* path, SPIFFS_Image& img) {
    SPIFFS_ImageReader reader;
    ImageReturnCode ret;
    ret = reader.loadBMP((char*)path, img);
    Serial.print("Load ");
    Serial.print(path);
    Serial.print(": ");
    reader.printStatus(ret);
    return ret == IMAGE_SUCCESS;
}

void Display::setFont(int num) {
    if (num < 0)
        Adafruit_SSD1351::setFont();
    else
        Adafruit_SSD1351::setFont(Fonts[num]);
}

void Display::printLeft(const char* str, int x, int y) {
    setCursor(x, y);
    print(str);
}

void Display::printCentered(const char* str, int x, int y) {
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
    setCursor(x - w / 2, y);
    print(str);
}

void Display::printRight(const char* str, int x, int y) {
    int16_t x1, y1;
    uint16_t w, h;
    getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
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
    if (y0 == y1) {
        fillRect(x0, y0 - (width / 2), x1 - x0, width, color);
    } else if (x0 == x1) {
        fillRect(x0 - (width / 2), y0, width, y1 - y0, color);
    } else {
        float dx = (width / 2.0) * (x0 - x1) / sqrt(sq(x0 - x1) + sq(y0 - y1));
        float dy = (width / 2.0) * (y0 - y1) / sqrt(sq(x0 - x1) + sq(y0 - y1));
        fillTriangle(x0 + dx, y0 - dy, x0 - dx, y0 + dy, x1 + dx, y1 - dy, color);
        fillTriangle(x0 - dx, y0 + dy, x1 - dx, y1 + dy, x1 + dx, y1 - dy, color);
    }
    if (rounded) {
        fillCircle(x0, y0, (uint16_t)(width / 2.0) - 1, color);
        fillCircle(x1, y1, (uint16_t)(width / 2.0) - 1, color);
    }
}

void Display::drawImage(SPIFFS_Image& image, int16_t x, int16_t y) {
    image.draw(*this, x, y);
}

void Display::drawImage(const char* file, int16_t x, int16_t y) {
    SPIFFS_ImageReader reader;
    unsigned long time = millis();
    //reader.drawBMP((char*)file, *this, x, y, false);
    reader.drawBMP((char*)file, *this, x, y);
    log_d("drawBMP %s: %d millis", file, millis() - time);
}


