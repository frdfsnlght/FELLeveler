#ifndef MAIN_SCREEN_H
#define MAIN_SCREEN_H

#include "screen.h"

class MainScreen : public Screen {

    public:

    static MainScreen* getInstance();

    String getName() { return "Main"; }
    void paintContent();

    private:

    static MainScreen* instance;

    MainScreen();

};

#endif
