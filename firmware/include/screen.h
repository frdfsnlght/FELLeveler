#ifndef SCREEN_H
#define SCREEN_H

#include "button.h"

class Screen {

    public:

    bool hidden = true;
    bool dirty = false;

    void show() {
        hidden = false;
        dirty = true;
        onShow();
    }

    void hide() {
        hidden = true;
        onHide();
    }

    virtual void onShow() {}
    virtual void onHide() {}
    virtual void handleButtonPress(Button* button) {}
    virtual void handleButtonRelease(Button* button) {}
    virtual void handleButtonLongPress(Button* button) {}
    virtual void handleButtonLongRelease(Button* button) {}

    virtual void paint() {}

    virtual void loop() {}

};

#endif
