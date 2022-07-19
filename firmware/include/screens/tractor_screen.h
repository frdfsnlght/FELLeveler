#ifndef TRACTOR_SCREEN_H
#define TRACTOR_SCREEN_H

#include "screen.h"

class TractorScreen : public Screen {

    public:

    static TractorScreen* getInstance();

    String getName() { return "Tractor"; }
    void paintContent();

    private:

    static TractorScreen* instance;

    TractorScreen();
    void handleButtonRelease(Button* button);

};

#endif
