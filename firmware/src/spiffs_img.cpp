#include "spiffs_img.h"

SPIFFS_Img::SPIFFS_Img() {}

SPIFFS_Img::~SPIFFS_Img() {
    dealloc();
}

void SPIFFS_Img::dealloc() {
    if (data != NULL) {
        free(data);
        data = NULL;
    }
    width = height = 0;
}

void SPIFFS_Img::draw(Adafruit_SPITFT &tft, int16_t x, int16_t y, uint16_t color) {
    if ((width == 0) || (height == 0)) return;
    if ((x >= tft.width()) || (y >= tft.height())) return;
    if (((x + width) < 0) || ((y + height) < 0)) return;

    int rowStart, rowEnd, colStart, colEnd;
    int winX, winY, winWidth, winHeight;
    uint8_t* nextBlock = data;
    uint16_t colorBuffer[MaxColorBuffer];
    int colorBufferLen = 0;
    RLEBlock block;
    uint16_t pixelColor;

    rowStart = colStart = 0;
    rowEnd = height - 1;
    colEnd = width - 1;
    winX = x;
    winY = y;
    winWidth = width;
    winHeight = height;

    // Crop for screen
    if (x < 0) {
        colStart = -x;
        winX = 0;
        winWidth += x;
    }
    if (y < 0) {
        rowStart = -y;
        winY = 0;
        winHeight += x;
    }
    if ((x + width) > tft.width()) {
        colEnd += tft.width() - (x + width);
        winWidth = tft.width() - x;
    }
    if ((y + height) > tft.height()) {
        rowEnd += tft.height() - (y + height);
        winHeight = tft.height() - y;
    }
    getRLEBlock(&nextBlock, block);
    pixelColor = getColor(color, block.value);

    tft.startWrite();
    tft.setAddrWindow(winX, winY, winWidth, winHeight);

    for (int row = 0; row < height; row++) {
        if (row > rowEnd) break;
        for (int col = 0; col < width; col++) {
            if (block.count == 0) {
                if (! getRLEBlock(&nextBlock, block)) {
                    log_e("Unexpected end of image data!");
                    tft.endWrite();
                    return;
                }
                pixelColor = getColor(color, block.value);
            }
            block.count--;
            if ((row >= rowStart) && (col >= colStart) && (col <= colEnd)) {
                colorBuffer[colorBufferLen++] = pixelColor;
                if (colorBufferLen == MaxColorBuffer) {
                    tft.writePixels(colorBuffer, colorBufferLen, true, false);
                    colorBufferLen = 0;
                    yield();
                }
            }
        }
    }
    if (colorBufferLen > 0) {
        tft.writePixels(colorBuffer, colorBufferLen, true, false);
    }
    tft.endWrite();
}

bool SPIFFS_Img::getRLEBlock(uint8_t** nextBlock, RLEBlock& block) {
    uint8_t byte;
    block.count = 0;
    while (true) {
        byte = **nextBlock;
        if (byte == 0) return false;
        *nextBlock++;
        if (byte & 0x80) {
            block.count = (block.count << 7) | (byte & 0x7f);
        } else {
            block.count = (block.count << 2) | ((byte & 0x60) >> 5);
            block.value = byte & 0x1f;
            return true;
        }
    }
}

uint16_t SPIFFS_Img::getColor(uint16_t color, uint8_t value) {
    if (value == 0) return 0;
    if (value == 0x1f) return color;
    float percent = (float)value / 31.0f;
    uint8_t red = (color & 0xf800) >> 11;
    uint8_t green = (color & 0x07e0) >> 5;
    uint8_t blue = (color & 0x001f);
    return
        ((uint16_t)((float)red * percent) << 11) |
        ((uint16_t)((float)green * percent) << 5) |
        (uint16_t)((float)blue * percent);
}

SPIFFS_ImgReader::SPIFFS_ImgReader() {}

SPIFFS_ImgReader::~SPIFFS_ImgReader() {}

SPIFFS_ImgReader::LoadResult SPIFFS_ImgReader::drawIMG(const char *filename, Adafruit_SPITFT &tft, int16_t x, int16_t y, uint16_t color) {
    if ((x >= tft.width()) || (y >= tft.height()))   // image is off-screen
        return Success;
    SPIFFS_Img img;
    LoadResult res = loadIMG(filename, img);
    if (res == Success)
        img.draw(tft, x, y, color);
    return res;
}

SPIFFS_ImgReader::LoadResult SPIFFS_ImgReader::loadIMG(const char *filename, SPIFFS_Img& img) {
    img.dealloc();
    File file;
    if (! (file = SPIFFS.open(filename, FILE_READ)))
        return FileNotFound;

    img.width = file.read();
    img.height = file.read();
    uint16_t dataSize = (file.read() << 8) | file.read();
    img.data = (uint8_t*)malloc(dataSize * sizeof(uint8_t));
    if (img.data == NULL) {
        file.close();
        return OutOfMemory;
    }
    size_t read = file.readBytes((char*)img.data, dataSize);
    file.close();
    if (read != dataSize)
        return BadFormat;
    return Success;
}

