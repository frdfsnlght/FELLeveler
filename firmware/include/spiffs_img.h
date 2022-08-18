// Library for loading and drawing tinted, 5 bit greyscale images.

#ifndef SPIFFS_IMG_H
#define SPIFFS_IMG_H

#include "SPIFFS.h"
#include "Adafruit_SPITFT.h"

class SPIFFS_Img {

    public:

    SPIFFS_Img();
    ~SPIFFS_Img();

    uint8_t width = 0;
    uint8_t height = 0;

    void draw(Adafruit_SPITFT &tft, int16_t x, int16_t y, uint16_t color);

    private:

    struct RLEBlock {
        uint16_t count;
        uint8_t value;
    };

    static const int MaxColorBuffer = 1024;

    uint8_t* data = NULL;

    void dealloc();
    bool getRLEBlock(uint8_t** nextBlock, RLEBlock& block);
    inline uint16_t getColor(uint16_t color, uint8_t value);
    
    friend class SPIFFS_ImgReader;
};

class SPIFFS_ImgReader {

    public:

    enum LoadResult {
        Success,
        FileNotFound,
        BadFormat,
        OutOfMemory
    };

    static const char* LoadResultStrings[];
    
    SPIFFS_ImgReader();
    ~SPIFFS_ImgReader();

    LoadResult drawIMG(const char *filename, Adafruit_SPITFT &tft, int16_t x, int16_t y, uint16_t color);
    LoadResult loadIMG(const char *filename, SPIFFS_Img& img);

};

#endif
