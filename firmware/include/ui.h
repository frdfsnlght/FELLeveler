#ifndef UI_H
#define UI_H

#include "screen.h"
#include "button.h"
#include <list>

class UI {

    public:

    static UI* getInstance();

    Button button = Button(12, Button::None, false, 0, 1000);
    
    void setup();
    void loop();

    void showScreen(Screen* newScreen);
    void nextScreen();
    void resetSave();
    
    private:

    static const int SaveStateDelay;
    static const char StateFile[];

    static UI* instance;
    Screen* screen = nullptr;
    bool saved = false;
    unsigned long lastSaveCheck;
    std::list<Screen*> screens;

    UI() {}

    void setupScreen(Screen* screen);
    bool restoreState();
    void saveState();

    void handleButtonPress(Button* button);
    void handleButtonRelease(Button* button);
    void handleButtonLongPress(Button* button);
    void handleButtonLongRelease(Button* button);

    friend class Screen;
    
};

#endif
