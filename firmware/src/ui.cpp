#include "ui.h"

UI* UI::instance = nullptr;

UI* UI::getInstance() {
    if (! instance) instance = new UI();
    return instance;
}

void UI::showScreen(Screen* newScreen) {
    if (newScreen == screen) return;
    if (screen != nullptr) {
        Serial.print("Hide screen: ");
        Serial.println(screen->getName());
        screen->hide();
    }
    screen = newScreen;
    if (screen != nullptr) {
        Serial.print("Show screen: ");
        Serial.println(screen->getName());
        screen->show(); 
    }
}

void UI::setup(Screen* startScreen) {
    button.setup();
    button.onPressListeners.add([](void) {
        UI::getInstance()->handleButtonPress(&UI::getInstance()->button);
    });
    button.onReleaseListeners.add([](void) {
        UI::getInstance()->handleButtonRelease(&UI::getInstance()->button);
    });
    button.onLongPressListeners.add([](void) {
        UI::getInstance()->handleButtonLongPress(&UI::getInstance()->button);
    });
    button.onLongReleaseListeners.add([](void) {
        UI::getInstance()->handleButtonLongRelease(&UI::getInstance()->button);
    });
    if (startScreen)
        showScreen(startScreen);
    Serial.println("UI setup complete");
}

void UI::loop() {
    button.loop();
    if (screen == nullptr) return;
    screen->loop();
    screen->paint();
}

void UI::handleButtonPress(Button* button) {
    if (screen == nullptr) return;
    screen->handleButtonPress(button);
};

void UI::handleButtonRelease(Button* button) {
    if (screen == nullptr) return;
    screen->handleButtonRelease(button);
};

void UI::handleButtonLongPress(Button* button) {
    if (screen == nullptr) return;
    screen->handleButtonLongPress(button);
};

void UI::handleButtonLongRelease(Button* button) {
    if (screen == nullptr) return;
    screen->handleButtonLongRelease(button);
};

