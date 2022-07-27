#ifndef IMPLEMENT_SCREEN_H
#define IMPLEMENT_SCREEN_H

#include <SPIFFS_ImageReader.h>

#include "screen.h"

class ImplementScreen : public Screen {

    public:

    static ImplementScreen* getInstance();

    void setup();
    String getName() { return "Implement"; }
    void paintContent();

    private:

    static ImplementScreen* instance;

    SPIFFS_Image rollLeft1Image;
    SPIFFS_Image rollLeft2Image;
    SPIFFS_Image rollLevelImage;
    SPIFFS_Image rollRight1Image;
    SPIFFS_Image rollRight2Image;

    SPIFFS_Image pitchUp1Image;
    SPIFFS_Image pitchUp2Image;
    SPIFFS_Image pitchLevelImage;
    SPIFFS_Image pitchDown1Image;
    SPIFFS_Image pitchDown2Image;

    void handleButtonRelease(Button* button);

};

#endif
