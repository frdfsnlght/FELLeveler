#ifndef UI_H
#define UI_H

#include "screen.h"
#include "button.h"

class UI {

    public:

    static UI* getInstance();

    Button button = Button(12, None, false, 0, 1000);
    
    void showScreen(Screen* newScreen);
    void setup(Screen* startScreen);
    void loop();

    private:

    static UI* instance;
    Screen* screen = nullptr;

    UI() {}

    static void handleButtonPress(Button* button);
    static void handleButtonRelease(Button* button);
    static void handleButtonLongPress(Button* button);
    static void handleButtonLongRelease(Button* button);

};

#endif
