#include "ConfigManager_RP2040.h"

ConfigManager::ConfigManager() {
    resetToDefaults();
}

void ConfigManager::begin() {
    EEPROM.begin(EEPROM_SIZE);

    if (loadConfig()) {
        Serial.println("[Config] Loaded from EEPROM");
        printConfig();
    } else {
        Serial.println("[Config] Using defaults");
        saveConfig();
    }
}

bool ConfigManager::loadConfig() {
    EEPROM.get(CONFIG_ADDRESS, config);

    if (!validateConfig()) {
        Serial.println("[Config] Invalid config, using defaults");
        resetToDefaults();
        return false;
    }

    return true;
}

bool ConfigManager::saveConfig() {
    config.magic = EEPROM_MAGIC;
    config.checksum = calculateChecksum();

    EEPROM.put(CONFIG_ADDRESS, config);
    EEPROM.commit();

    Serial.println("[Config] Saved to EEPROM");
    return true;
}

void ConfigManager::resetToDefaults() {
    config.magic = EEPROM_MAGIC;

    // PID defaults
    config.pid.kp = DEFAULT_KP;
    config.pid.ki = DEFAULT_KI;
    config.pid.kd = DEFAULT_KD;
    config.pid.setpoint = DEFAULT_SETPOINT;
    config.pid.autoMode = true;

    // Sensor defaults
    config.sensor.activeSensor = 0; // SENSOR_NONE
    config.sensor.calOffset = 0.0;
    config.sensor.calScale = 1.0;
    config.sensor.calEnabled = false;

    // Relay defaults
    config.relay.mode = RELAY_OFF;
    config.relay.hysteresis = RELAY_HYSTERESIS;
    config.relay.minCycleTime = RELAY_MIN_CYCLE;
    config.relay.timeProportioning = false;
    config.relay.windowSize = 5000;

    config.checksum = calculateChecksum();

    Serial.println("[Config] Reset to defaults");
}

uint16_t ConfigManager::calculateChecksum() {
    uint16_t sum = 0;
    uint8_t* data = (uint8_t*)&config;

    // Checksum all data except the checksum field itself
    size_t size = sizeof(SystemConfig) - sizeof(config.checksum);

    for (size_t i = 0; i < size; i++) {
        sum += data[i];
    }

    return sum;
}

bool ConfigManager::validateConfig() {
    if (config.magic != EEPROM_MAGIC) {
        Serial.println("[Config] Invalid magic number");
        return false;
    }

    uint16_t calculatedChecksum = calculateChecksum();
    if (config.checksum != calculatedChecksum) {
        Serial.printf("[Config] Checksum mismatch: %04X != %04X\n",
                      config.checksum, calculatedChecksum);
        return false;
    }

    return true;
}

void ConfigManager::setPIDConfig(const PIDConfig& pidCfg) {
    config.pid = pidCfg;
    saveConfig();
}

void ConfigManager::setPIDTunings(double kp, double ki, double kd) {
    config.pid.kp = kp;
    config.pid.ki = ki;
    config.pid.kd = kd;
    saveConfig();
}

void ConfigManager::setSetpoint(double setpoint) {
    config.pid.setpoint = setpoint;
    saveConfig();
}

void ConfigManager::setAutoMode(bool mode) {
    config.pid.autoMode = mode;
    saveConfig();
}

void ConfigManager::setSensorConfig(const SensorConfig& sensorCfg) {
    config.sensor = sensorCfg;
    saveConfig();
}

void ConfigManager::setActiveSensor(uint8_t sensor) {
    config.sensor.activeSensor = sensor;
    saveConfig();
}

void ConfigManager::setCalibration(float offset, float scale) {
    config.sensor.calOffset = offset;
    config.sensor.calScale = scale;
    saveConfig();
}

void ConfigManager::enableCalibration(bool enable) {
    config.sensor.calEnabled = enable;
    saveConfig();
}

void ConfigManager::setRelayConfig(const RelayConfig& relayCfg) {
    config.relay = relayCfg;
    saveConfig();
}

void ConfigManager::setRelayMode(uint8_t mode) {
    config.relay.mode = mode;
    saveConfig();
}

void ConfigManager::setHysteresis(float value) {
    config.relay.hysteresis = value;
    saveConfig();
}

void ConfigManager::printConfig() {
    Serial.println("\n========== Configuration ==========");
    Serial.printf("Magic: 0x%04X\n", config.magic);
    Serial.printf("Checksum: 0x%04X\n", config.checksum);

    Serial.println("\nPID:");
    Serial.printf("  Kp: %.2f\n", config.pid.kp);
    Serial.printf("  Ki: %.2f\n", config.pid.ki);
    Serial.printf("  Kd: %.2f\n", config.pid.kd);
    Serial.printf("  Setpoint: %.2f\n", config.pid.setpoint);
    Serial.printf("  Auto Mode: %s\n", config.pid.autoMode ? "YES" : "NO");

    Serial.println("\nSensor:");
    Serial.printf("  Active: %d\n", config.sensor.activeSensor);
    Serial.printf("  Cal Offset: %.2f\n", config.sensor.calOffset);
    Serial.printf("  Cal Scale: %.3f\n", config.sensor.calScale);
    Serial.printf("  Cal Enabled: %s\n", config.sensor.calEnabled ? "YES" : "NO");

    Serial.println("\nRelay:");
    Serial.printf("  Mode: %d\n", config.relay.mode);
    Serial.printf("  Hysteresis: %.2f\n", config.relay.hysteresis);
    Serial.printf("  Min Cycle: %d ms\n", config.relay.minCycleTime);

    Serial.println("===================================\n");
}
