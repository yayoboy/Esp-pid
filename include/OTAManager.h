#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "config.h"

class OTAManager {
private:
    bool otaInProgress;

public:
    OTAManager();
    void begin();
    void handle();
    bool isInProgress() const { return otaInProgress; }
};

#endif // OTA_MANAGER_H
