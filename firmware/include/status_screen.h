#ifndef STATUS_SCREEN_H
#define STATUS_SCREEN_H

#include "screen.h"

class StatusScreen : public Screen {

    public:

    static StatusScreen* getInstance();

    String getName() { return "Status"; }
    void paint();

    private:

    static StatusScreen* instance;

    static void statusUpdated(int);

    StatusScreen();


};

#endif