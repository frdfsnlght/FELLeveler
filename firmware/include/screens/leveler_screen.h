#ifndef LEVELER_SCREEN_H
#define LEVELER_SCREEN_H

#include <SPIFFS_ImageReader.h>

#include "screen.h"

class LevelerScreen : public Screen {

    public:

    static LevelerScreen* getInstance();

    void setup();
    String getName() { return "Leveler"; }
    void paintContent();

    private:

    static LevelerScreen* instance;
    static const char* ModeStrings[];

    enum Mode {
        Tractor,
        Earth
    };

    Mode mode;
    SPIFFS_Image leftImage;
    SPIFFS_Image rightImage;
    SPIFFS_Image levelImage;
    SPIFFS_Image noneImage;

    void handleButtonRelease(Button* button);

};

#endif
