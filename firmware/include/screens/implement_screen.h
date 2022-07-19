#ifndef IMPLEMENT_SCREEN_H
#define IMPLEMENT_SCREEN_H

#include "screen.h"

class ImplementScreen : public Screen {

    public:

    static ImplementScreen* getInstance();

    String getName() { return "Implement"; }
    void paintContent();

    private:

    static ImplementScreen* instance;
    static const char* ModeStrings[];

    enum Mode {
        Tractor,
        Earth
    };

    Mode mode;

    ImplementScreen();
    void handleButtonRelease(Button* button);

};

#endif
