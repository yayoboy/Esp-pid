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

struct PinConfig {
    // Sensor pins
    uint8_t dht_pin;
    uint8_t bme_sda;
    uint8_t bme_scl;
    uint8_t encoder_a;
    uint8_t encoder_b;
    uint8_t thermo_cs;
    uint8_t thermo_sck;
    uint8_t thermo_miso;
    uint8_t ds18b20_pin;

    // Output pins
    uint8_t pwm_output;
    uint8_t relay_1;
    uint8_t relay_2;
    uint8_t relay_3;
    uint8_t relay_4;

    // Display pins (TFT)
    uint8_t tft_cs;
    uint8_t tft_dc;
    uint8_t tft_rst;
    uint8_t tft_mosi;
    uint8_t tft_sclk;
    uint8_t tft_miso;

    // Display pins (OLED)
    uint8_t oled_sda;
    uint8_t oled_scl;
};

class ConfigManager {
private:
    PIDConfig pidConfig;
    WiFiConfig wifiConfig;
    PinConfig pinConfig;

public:
    ConfigManager();
    bool begin();
    bool loadConfig();
    bool saveConfig();

    // PID Config
    PIDConfig getPIDConfig() const { return pidConfig; }
    void setPIDConfig(const PIDConfig& config);
    void setPIDTunings(double kp, double ki, double kd);
    void setSetpoint(double setpoint);
    void setAutoMode(bool mode);

    // WiFi Config
    WiFiConfig getWiFiConfig() const { return wifiConfig; }
    void setWiFiConfig(const WiFiConfig& config);

    // Pin Config
    PinConfig getPinConfig() const { return pinConfig; }
    void setPinConfig(const PinConfig& config);
    bool validatePin(uint8_t pin, bool allowInputOnly = false);

    // Individual pin setters
    void setSensorPin(const char* sensor, uint8_t pin);
    void setOutputPin(const char* output, uint8_t pin);
    void setDisplayPin(const char* display, uint8_t pin);
};

#endif // CONFIG_MANAGER_H
