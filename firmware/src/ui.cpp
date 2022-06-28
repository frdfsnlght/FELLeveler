#include "ui.h"

UI* UI::instance = nullptr;

UI* UI::getInstance() {
    if (! instance) instance = new UI();
    return instance;
}

void UI::showScreen(Screen* newScreen) {
    if (newScreen == screen) return;
    if (screen != nullptr)
        screen->hide();
    screen = newScreen;
    screen->show(); 
}

void UI::setup(Screen* startScreen) {
    button.begin();
    button.onPressListeners.add(handleButtonPress);
    button.onReleaseListeners.add(handleButtonRelease);
    button.onLongPressListeners.add(handleButtonLongPress);
    button.onLongReleaseListeners.add(handleButtonLongRelease);
    if (startScreen)
        showScreen(startScreen);
    Serial.println("UI setup complete");
}

void UI::loop() {
    button.update();
    if (screen == nullptr) return;
    screen->loop();
    if (screen->dirty)  {
        screen->paint();
        screen->dirty = false;
    }
}

void UI::handleButtonPress(Button* button) {
    UI* ui = getInstance();
    if (ui->screen == nullptr) return;
    ui->screen->handleButtonPress(button);
};

void UI::handleButtonRelease(Button* button) {
    UI* ui = getInstance();
    if (ui->screen == nullptr) return;
    ui->screen->handleButtonRelease(button);
};

void UI::handleButtonLongPress(Button* button) {
    UI* ui = getInstance();
    if (ui->screen == nullptr) return;
    ui->screen->handleButtonLongPress(button);
};

void UI::handleButtonLongRelease(Button* button) {
    UI* ui = getInstance();
    if (ui->screen == nullptr) return;
    ui->screen->handleButtonLongRelease(button);
};
