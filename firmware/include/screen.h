#ifndef SCREEN_H
#define SCREEN_H

#include "button.h"

class Screen {

    public:

    bool hidden = true;
    bool dirty = false;
    bool firstPaint = false;
    uint16_t backgroundColor = 0;

    void show();
    void hide();

    virtual String getName();
    virtual void onShow() {}
    virtual void onHide() {}
    virtual void handleButtonPress(Button* button) {}
    virtual void handleButtonRelease(Button* button) {}
    virtual void handleButtonLongPress(Button* button) {}
    virtual void handleButtonLongRelease(Button* button) {}

    virtual void paint();
    virtual void paintContent() {}
    
    virtual void loop() {}

    protected:

    virtual void paintBackground();

};

#endif
