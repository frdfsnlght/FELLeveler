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
    if (data == NULL) return;
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

    //log_d("colStart=%d, colEnd=%d, rowStart=%d, rowEnd=%d, winX=%d, winY=%d, winWidth=%d, winHeight=%d",
    //            colStart, colEnd, rowStart, rowEnd, winX, winY, winWidth, winHeight);

    getRLEBlock(&nextBlock, block);
    //log_d("first block: count=%d, value=%d", block.count, block.value);

    pixelColor = getColor(color, block.value);
    //log_d("first color: %#04x", pixelColor);

    tft.startWrite();
    tft.setAddrWindow(winX, winY, winWidth, winHeight);

    for (y = 0; y <= rowEnd; y++) {
        for (x = 0; x < width; x++) {
            if (block.count == 0) {
                if (! getRLEBlock(&nextBlock, block)) {
                    log_e("Unexpected end of image data!");
                    tft.endWrite();
                    return;
                }
                pixelColor = getColor(color, block.value);
            }
            block.count--;
            if ((y >= rowStart) && (x >= colStart) && (x <= colEnd)) {
                colorBuffer[colorBufferLen++] = pixelColor;
                if (colorBufferLen == MaxColorBuffer) {
                    //log_d("writing colorBuffer");
                    tft.writePixels(colorBuffer, colorBufferLen, true, false);
                    colorBufferLen = 0;
                    yield();
                }
            }
        }
    }
    if (colorBufferLen > 0) {
        //log_d("writing remaining colorBuffer (%d)", colorBufferLen);
        tft.writePixels(colorBuffer, colorBufferLen, true, false);
    }
    tft.endWrite();
}

bool SPIFFS_Img::getRLEBlock(uint8_t** nextBlock, RLEBlock& block) {
    uint8_t byte;
    block.count = 0;
    byte = **nextBlock;
    if (byte == 0) return false;
    //log_d("*nextBlock=%x, byte=%02x", *nextBlock, byte);
    (*nextBlock)++;

    while (true) {
        if (byte & 0x80) {
            block.count = (block.count << 7) | (byte & 0x7f);
            //log_d("count=%d, getting next byte", block.count);
        } else {
            block.count = (block.count << 2) | ((byte & 0x60) >> 5);
            block.value = byte & 0x1f;
            //log_d("count=%d, value=%d", block.count, block.value);
            return true;
        }
        byte = **nextBlock;
        //log_d("byte=%02x", byte);
        (*nextBlock)++;
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

const char* SPIFFS_ImgReader::LoadResultStrings[] = {
    "Success",
    "FileNotFound",
    "BadFormat",
    "OutOfMemory"
};

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
    unsigned long time = millis();
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
    //log_d("%s: %d millis, %d data bytes", filename, millis() - time, dataSize);
    return Success;
}

