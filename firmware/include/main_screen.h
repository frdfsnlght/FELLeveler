#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include "screen.h"

class MainScreen : public Screen {

    public:

    static MainScreen* getInstance();

    void paint();

    private:

    static MainScreen* instance;

    MainScreen();

};

#endif
