#include "ui.h"

#include "config.h"

#include "screens/boot_screen.h"
#include "screens/tractor_screen.h"
#include "screens/implement_screen.h"
#include "screens/leveler_screen.h"
#include "screens/status_screen.h"

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

    // Setup all screens
    BootScreen::getInstance()->setup();
    TractorScreen::getInstance()->setup();
    ImplementScreen::getInstance()->setup();
    LevelerScreen::getInstance()->setup();
    StatusScreen::getInstance()->setup();

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

void UI::nextScreen() {
    Config::Mode mode = Config::getInstance()->mode;

    if (screen == NULL) {
        showScreen(BootScreen::getInstance());
        return;
    }

    if (screen == BootScreen::getInstance()) {
        if (mode == Config::Tractor)
            showScreen(TractorScreen::getInstance());
        else if (mode == Config::Implement)
            showScreen(ImplementScreen::getInstance());

    } else if (screen == TractorScreen::getInstance()) {
        if (mode == Config::Tractor)
            showScreen(ImplementScreen::getInstance());
        else if (mode == Config::Implement)
            showScreen(StatusScreen::getInstance());

    } else if (screen == ImplementScreen::getInstance()) {
        if (mode == Config::Tractor)
            showScreen(LevelerScreen::getInstance());
        else if (mode == Config::Implement)
            showScreen(StatusScreen::getInstance());

    } else if (screen == LevelerScreen::getInstance()) {
        showScreen(StatusScreen::getInstance());

    } else if (screen == StatusScreen::getInstance()) {
        if (mode == Config::Tractor)
            showScreen(TractorScreen::getInstance());
        else if (mode == Config::Implement)
            showScreen(ImplementScreen::getInstance());
    }

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

