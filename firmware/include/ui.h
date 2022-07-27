#ifndef UI_H
#define UI_H

#include "screen.h"
#include "button.h"

class UI {

    public:

    static UI* getInstance();

    Button button = Button(12, Button::None, false, 0, 1000);
    
    void showScreen(Screen* newScreen);
    void setup(Screen* startScreen);
    void loop();

    void nextScreen();
    
    private:

    static UI* instance;
    Screen* screen = nullptr;

    UI() {}

    void handleButtonPress(Button* button);
    void handleButtonRelease(Button* button);
    void handleButtonLongPress(Button* button);
    void handleButtonLongRelease(Button* button);

};

#endif
