#ifndef BOOT_SCREEN_H
#define BOOT_SCREEN_H

#include "screen.h"
#include "ui.h"

class BootScreen : public Screen {

    public:

    static BootScreen* getInstance();

    void onShow();
    void loop();
    void paint();

    private:

    static BootScreen* instance;

    const unsigned long ShowTime = 3000;
    unsigned long time;

    BootScreen() {}

};

#endif
