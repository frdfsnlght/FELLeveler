#ifndef STATUS_SCREEN_H
#define STATUS_SCREEN_H

#include "screen.h"

class StatusScreen : public Screen {

    public:

    static StatusScreen* getInstance();

    String getName() { return "Status"; }
    void loop();

    protected:

    void paintContent();
    void handleButtonRelease(Button* button);

    private:

    static StatusScreen* instance;

    struct DirtyFlags {
        bool network : 1;
        bool angles : 1;
        bool remote : 1;
        bool remoteAngles : 1;
        bool memory : 1;
    };

    DirtyFlags dirtyFlags;

    static void statusUpdated(int);

    StatusScreen();

};

#endif
