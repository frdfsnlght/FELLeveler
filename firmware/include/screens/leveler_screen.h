#ifndef LEVELER_SCREEN_H
#define LEVELER_SCREEN_H

//#include <SPIFFS_ImageReader.h>

#include "screen.h"
#include "display.h"

class LevelerScreen : public Screen {

    public:

    static LevelerScreen* getInstance();

    String getName() { return "Leveler"; }
    void setup();
    String saveState();
    void restoreState(String& state);
    bool canShow();
    void onShow();
    void paintContent();

    private:


    static LevelerScreen* instance;
    static const char* ModeStrings[];
    static const uint8_t BoxWidth = 110;
    static const uint8_t BoxHeight = 110;
    static const uint8_t LineWidth = 16;

    struct DirtyFlags {
        bool angle : 1;
        bool image : 1;
        bool mode : 1;
    };
    
    enum Mode {
        Tractor,
        Earth
    };

    Mode mode;
    int diff;
    int image;
    DirtyFlags dirtyFlags;

    void handleButtonRelease(Button* button);
    void update();

    void drawRight(Display* display, int x0, int y0, int x1, int y1);
    void drawLeft(Display* display, int x0, int y0, int x1, int y1);
    void drawLevel(Display* d, int x0, int y0, int x1, int y1);
    void drawNone(Display* d, int x0, int y0, int x1, int y1);

};

#endif
