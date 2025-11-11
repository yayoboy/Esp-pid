#include "OTAManager.h"

OTAManager::OTAManager() : otaInProgress(false) {
}

void OTAManager::begin() {
    ArduinoOTA.setHostname(HOSTNAME);
    ArduinoOTA.setPassword("admin");

    ArduinoOTA.onStart([this]() {
        otaInProgress = true;
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([this]() {
        otaInProgress = false;
        Serial.println("\nOTA Update complete");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        otaInProgress = false;
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });

    ArduinoOTA.begin();
    Serial.println("OTA Ready");
}

void OTAManager::handle() {
    ArduinoOTA.handle();
}
