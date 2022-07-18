#ifndef STATUS_SCREEN_H
#define STATUS_SCREEN_H

#include "screen.h"

class StatusScreen : public Screen {

    public:

    static StatusScreen* getInstance();

    String getName() { return "Status"; }

    protected:

    void paintContent();

    private:

    static StatusScreen* instance;

    struct DirtyFlags {
        bool network : 1;
        bool netsock : 1;
        bool levelerRoll : 1;
        bool levelerPitch : 1;
        bool levelerImplementRoll : 1;
        bool levelerImplementPitch : 1;
    };

    DirtyFlags dirtyFlags;

    static void statusUpdated(int);

    StatusScreen();

    void networkChanged();
    void netsockChanged();
    void rollChanged();
    void pitchChanged();
    void implementRollChanged();
    void implementPitchChanged();

};

#endif
