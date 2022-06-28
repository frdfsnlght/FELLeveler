#ifndef UI_H
#define UI_H

#include "screen.h"
#include "button.h"

class UI {

    public:

    static UI* getInstance();
    void showScreen(Screen* newScreen);
    void setup(Screen* startScreen);
    void loop();

    private:

    static UI* instance;
    Button button = Button(13, Pullup, false, 0, 1000);
    Screen* screen;

    UI() {}

    static void handleButtonPress(Button* button);
    static void handleButtonRelease(Button* button);
    static void handleButtonLongPress(Button* button);
    static void handleButtonLongRelease(Button* button);

};

#endif
