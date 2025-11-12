#ifndef CONFIG_MANAGER_RP2040_H
#define CONFIG_MANAGER_RP2040_H

#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

struct PIDConfig {
    double kp;
    double ki;
    double kd;
    double setpoint;
    bool autoMode;
};

struct SensorConfig {
    uint8_t activeSensor;  // SensorType enum value
    float calOffset;
    float calScale;
    bool calEnabled;
};

struct RelayConfig {
    uint8_t mode;  // RelayMode enum value
    float hysteresis;
    uint16_t minCycleTime;
    bool timeProportioning;
    uint16_t windowSize;
};

struct SystemConfig {
    uint16_t magic;  // Magic number for validation
    PIDConfig pid;
    SensorConfig sensor;
    RelayConfig relay;
    uint16_t checksum;
};

class ConfigManager {
private:
    SystemConfig config;

    uint16_t calculateChecksum();
    bool validateConfig();

public:
    ConfigManager();
    void begin();

    // Load/Save
    bool loadConfig();
    bool saveConfig();
    void resetToDefaults();

    // PID Config
    PIDConfig getPIDConfig() const { return config.pid; }
    void setPIDConfig(const PIDConfig& pidCfg);
    void setPIDTunings(double kp, double ki, double kd);
    void setSetpoint(double setpoint);
    void setAutoMode(bool mode);

    // Sensor Config
    SensorConfig getSensorConfig() const { return config.sensor; }
    void setSensorConfig(const SensorConfig& sensorCfg);
    void setActiveSensor(uint8_t sensor);
    void setCalibration(float offset, float scale);
    void enableCalibration(bool enable);

    // Relay Config
    RelayConfig getRelayConfig() const { return config.relay; }
    void setRelayConfig(const RelayConfig& relayCfg);
    void setRelayMode(uint8_t mode);
    void setHysteresis(float value);

    // System info
    void printConfig();
};

#endif // CONFIG_MANAGER_RP2040_H
