#include "screens/boot_screen.h"

#include <SPIFFS.h>

#include "ui.h"
#include "display.h"

#include "screens/implement_screen.h"

BootScreen* BootScreen::instance = nullptr;

BootScreen* BootScreen::getInstance() {
    if (instance == nullptr) instance = new BootScreen();
    return instance;
}

BootScreen::BootScreen() : Screen() {
    // load images
    SPIFFS_ImageReader reader;
    ImageReturnCode ret;
    
    ret = reader.loadBMP((char*)"/logo.bmp", logoImage);
    Serial.print("Load /logo.bmp: "); reader.printStatus(ret);
}

void BootScreen::onShow() {
    time = millis();
}

void BootScreen::loop() {
    if ((millis() - time) > ShowTime) {
        UI::getInstance()->showScreen(ImplementScreen::getInstance());
    }
}

void BootScreen::paintContent() {
    Display* d = Display::getInstance();
    d->setFont(1);
    d->setTextColor(WHITE);
    d->printCentered("FELLeveler", d->width() / 2, d->height() / 2);
    d->drawImage(logoImage, (d->width() - logoImage.width()) / 2, (d->height() / 2) + 4);
}

