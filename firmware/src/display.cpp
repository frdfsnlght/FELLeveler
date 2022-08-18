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

void Display::drawImg(SPIFFS_Img& image, int16_t x, int16_t y, uint16_t color) {
    unsigned long time = millis();
    image.draw(*this, x, y, color);
    log_d("%d millis", millis() - time);
}

bool Display::drawImg(const char* file, int16_t x, int16_t y, uint16_t color) {
    SPIFFS_ImgReader reader;
    unsigned long time = millis();
    SPIFFS_ImgReader::LoadResult res = reader.drawIMG((char*)file, *this, x, y, color);
    if (res != SPIFFS_ImgReader::Success)
        log_e("%s: %s", file, SPIFFS_ImgReader::LoadResultStrings[res]);
    else
        log_d("%s: %d millis", file, millis() - time);
    return res == SPIFFS_ImgReader::Success;
}

bool Display::loadImg(const char* file, SPIFFS_Img& img) {
    SPIFFS_ImgReader reader;
    unsigned long time = millis();
    SPIFFS_ImgReader::LoadResult res = reader.loadIMG((char*)file, img);
    if (res != SPIFFS_ImgReader::Success)
        log_e("%s: %s", file, SPIFFS_ImgReader::LoadResultStrings[res]);
    else
        log_d("%s: %d millis", file, millis() - time);
    return res == SPIFFS_ImgReader::Success;
}
