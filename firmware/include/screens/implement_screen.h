#ifndef IMPLEMENT_SCREEN_H
#define IMPLEMENT_SCREEN_H

#include "screen.h"
#include "spiffs_img.h"

class ImplementScreen : public Screen {

    public:

    static ImplementScreen* getInstance();

    String getName() { return "Implement"; }
    void setup();
    void onShow();
    void paintContent();

    private:

    struct DirtyFlags {
        bool roll : 1;
        bool rollImage : 1;
        bool pitch : 1;
        bool pitchImage : 1;
    };

    static ImplementScreen* instance;
    static const int MaxImages = 10;
    static const char* ImageFiles[];
    static SPIFFS_Img Images[MaxImages];
    static const uint16_t Colors[];

    int roll;
    int pitch;
    int rollImage;
    int pitchImage;
    DirtyFlags dirtyFlags;

    void handleButtonRelease(Button* button);
    void update();

};

#endif
