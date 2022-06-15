#ifndef OTA_H
#define OTA_H

const int OTA_PORT = 3232;
const char* OTA_HOSTNAME = "felleveler";

int otaUpdateType = 0;

void setupOTA() {
    ArduinoOTA.setPort(OTA_PORT);  
    ArduinoOTA.setHostname(OTA_HOSTNAME);
    ArduinoOTA.setRebootOnSuccess(true);
  
    // No authentication by default
    // ArduinoOTA.setPassword("admin");
    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
    ArduinoOTA.onStart([]() {
        otaUpdateType = ArduinoOTA.getCommand();
        if (otaUpdateType == U_FLASH)
            Serial.println("Updating flash");
        else {
            Serial.println("Updating spiffs");
            SPIFFS.end();
        }
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA setup complete");
}

void loopOTA() {
    ArduinoOTA.handle();
}

#endif
