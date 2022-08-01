#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <SPIFFS_ImageReader.h>

#define SPI_PORT 3

// SPI2 port
// MOSI - 13
// MISO - 12
// SCLK - 14
// CS - 15

// SPI3 port
// MOSI - 23
// MISO - 19
// SCLK - 18
// CS - 5

#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

class Display : public Adafruit_SSD1351 {

    public:

    static constexpr int Width = 128;
    static constexpr int Height = 128;

    static Display* getInstance();

    void setup();

    bool loadImage(const char* path, SPIFFS_Image& img);

    void setFont(int num);
    void printLeft(const char* str, int x, int y);
    void printCentered(const char* str, int x, int y);
    void printRight(const char* str, int x, int y);
    void fillRight(uint16_t color);

    void drawThickLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color, bool rounded = false);
    
    void drawImage(SPIFFS_Image& image, int16_t x, int16_t y);
    void drawImage(const char* file, int16_t x, int16_t y);

    private:

    static const GFXfont* Fonts[];

#if SPI_PORT == 2
    static constexpr int CSPin = 15;
    static constexpr int MOSIPin = 13;
    static constexpr int MISOPin = 12;
    static constexpr int SCLKPin = 14;
    static constexpr int DCPin = 17;    // pick a pin
    static constexpr int RSTPin = 16;   // pick a pin, or don't use
#elif SPI_PORT == 3
    static constexpr int CSPin = 5;
    static constexpr int MOSIPin = 23;
    static constexpr int MISOPin = 19;
    static constexpr int SCLKPin = 18;
    static constexpr int DCPin = 17;    // pick a pin
    static constexpr int RSTPin = 16;   // pick a pin, or don't use
#else
    #warning "Invalid or no SPI port selected for display!"
#endif

    static Display* instance;

    //Hardware SPI
#if SPI_PORT == 2
    Display() : Adafruit_SSD1351(Width, Height, new SPIClass(HSPI), CSPin, DCPin) {}
#elif SPI_PORT == 3
    Display() : Adafruit_SSD1351(Width, Height, new SPIClass(VSPI), CSPin, DCPin) {}
#endif

};

#endif