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

    int imgX, imgY, imgWidth, imgHeight;
    int blockX, blockY;
    nextBlock = data;
    RLEBlock block;
    uint16_t pixelColor;

    imgX = 0;
    imgY = 0;
    imgWidth = width;
    imgHeight = height;

    // Crop for screen
    if (x < 0) {
        imgX = -x;
        imgWidth += x;
        x = 0;
    }
    if (y < 0) {
        imgY = -y;
        imgHeight += y;
        y = 0;
    }
    if ((x + imgWidth) > tft.width())
        imgWidth = tft.width() - x;
    if ((y + imgHeight) > tft.height())
        imgHeight = tft.height() - y;

    blockX = 0;
    blockY = 0;
    getRLEBlock(block);
    pixelColor = getColor(color, block.value);
    uint16_t buffer[MaxPixelBuffer];
    int bufferLen = 0;

    tft.startWrite();
    tft.setAddrWindow(x, y, imgWidth, imgHeight);

    for (int row = 0; row < imgHeight; row++) {
        for (int col = 0; col < imgWidth; col++) {
            

        }

    //while (getRLEBlock(block)) {
//        yield();
//        pixelColor = getColor(color, block.value);

      //                  tft->writePixels(dest, destidx, true);

    }
    tft.dmaWait();
    tft.endWrite();
}

bool SPIFFS_Img::getRLEBlock(RLEBlock& block) {
    uint8_t byte;
    block.count = 0;
    while (true) {
        byte = *nextBlock;
        if (byte == 0) return false;
        nextBlock++;
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

