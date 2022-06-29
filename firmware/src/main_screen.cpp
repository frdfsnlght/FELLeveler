#include "main_screen.h"

MainScreen* MainScreen::instance = nullptr;

MainScreen* MainScreen::getInstance() {
    if (instance == nullptr) instance = new MainScreen();
    return instance;
}

MainScreen::MainScreen() : Screen() {
    // TODO: add listeners
}

void MainScreen::paint() {
    // TODO
}
