#ifndef BOOT_SCREEN_H
#define BOOT_SCREEN_H

#include "screen.h"

class BootScreen : public Screen {

    public:

    static BootScreen* getInstance();

    String getName() { return "Boot"; }
    void onShow();
    void loop();
    void paintContent();

    private:

    static BootScreen* instance;

    const unsigned long ShowTime = 5000;
    unsigned long time = 0;

    BootScreen() {}

};

#endif
