#include "ui.h"

#include <SPIFFS.h>

#include "config.h"

#include "screens/boot_screen.h"
#include "screens/tractor_screen.h"
#include "screens/implement_screen.h"
#include "screens/leveler_screen.h"
#include "screens/status_screen.h"

UI* UI::instance = nullptr;
const int UI::SaveStateDelay = 10000;
const char UI::StateFile[] = "/ui.state";

UI* UI::getInstance() {
    if (! instance) instance = new UI();
    return instance;
}

void UI::setup() {
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
    setupScreen(BootScreen::getInstance());
    setupScreen(TractorScreen::getInstance());
    setupScreen(ImplementScreen::getInstance());
    setupScreen(LevelerScreen::getInstance());
    setupScreen(StatusScreen::getInstance());

    Serial.println("UI setup complete");
}

void UI::loop() {
    button.loop();

    if (screen == nullptr) return;
    screen->loop();
    screen->paint();

    if ((! saved) && ((millis() - lastSaveCheck) > SaveStateDelay) && (screen != BootScreen::getInstance())) {
        lastSaveCheck = millis();
        saveState();
    }
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
        resetSave();
    }
}

void UI::nextScreen() {
    Config::Mode mode = Config::getInstance()->running.mode;

    if (screen == NULL) {
        showScreen(BootScreen::getInstance());
        return;
    }

    if (screen == BootScreen::getInstance()) {
        if (! restoreState()) {
            if (mode == Config::Tractor)
                showScreen(TractorScreen::getInstance());
            else if (mode == Config::Implement)
                showScreen(ImplementScreen::getInstance());
        }

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

void UI::setupScreen(Screen* screen) {
    screen->setup();
    screens.push_back(screen);
}

bool UI::restoreState() {
    File file = SPIFFS.open(StateFile, FILE_READ);
    if (! file) {
        log_d("Unable to read state file %s", StateFile);
        return false;
    }
    String screenName = file.readStringUntil('\n');
    String screenState = file.readStringUntil('\n');
    file.close();
    screenName.trim();
    screenState.trim();
    for (Screen* screen : screens) {
        if (screenName.equals(screen->getName())) {
            if (! screen->canShow()) break;
            showScreen(screen);
            screen->restoreState(screenState);
            saved = true;
            Serial.printf("Restored screen\n");
            return true;
        }
    }
    Serial.printf("Unable to restore screen %s\n", screenName.c_str());
    return false;
}

void UI::saveState() {
    saved = true;
    if (screen == NULL) return;
    File file = SPIFFS.open(StateFile, FILE_WRITE);
    if (! file) {
        log_e("Unable to write state file %s", StateFile);
        return;
    }
    file.println(screen->getName());
    file.println(screen->saveState());
    file.close();
    Serial.printf("Saved state to %s\n", StateFile);
}

void UI::resetSave() {
    saved = false;
    lastSaveCheck = millis();
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

