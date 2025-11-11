#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LITTLEFS.h>
#include "config.h"

struct PIDConfig {
    double kp;
    double ki;
    double kd;
    double setpoint;
    bool autoMode;
};

struct WiFiConfig {
    char ssid[32];
    char password[64];
    char hostname[32];
};

class ConfigManager {
private:
    PIDConfig pidConfig;
    WiFiConfig wifiConfig;

public:
    ConfigManager();
    bool begin();
    bool loadConfig();
    bool saveConfig();

    PIDConfig getPIDConfig() const { return pidConfig; }
    WiFiConfig getWiFiConfig() const { return wifiConfig; }

    void setPIDConfig(const PIDConfig& config);
    void setWiFiConfig(const WiFiConfig& config);

    void setPIDTunings(double kp, double ki, double kd);
    void setSetpoint(double setpoint);
    void setAutoMode(bool mode);
};

#endif // CONFIG_MANAGER_H
