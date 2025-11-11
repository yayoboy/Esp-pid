#include "ConfigManager.h"

ConfigManager::ConfigManager() {
    // Set defaults
    pidConfig.kp = DEFAULT_KP;
    pidConfig.ki = DEFAULT_KI;
    pidConfig.kd = DEFAULT_KD;
    pidConfig.setpoint = DEFAULT_SETPOINT;
    pidConfig.autoMode = true;

    strncpy(wifiConfig.ssid, WIFI_SSID, sizeof(wifiConfig.ssid));
    strncpy(wifiConfig.password, WIFI_PASSWORD, sizeof(wifiConfig.password));
    strncpy(wifiConfig.hostname, HOSTNAME, sizeof(wifiConfig.hostname));
}

bool ConfigManager::begin() {
    if (!LITTLEFS.begin(true)) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    Serial.println("LittleFS mounted");

    return loadConfig();
}

bool ConfigManager::loadConfig() {
    if (!LITTLEFS.exists(CONFIG_FILE)) {
        Serial.println("Config file not found, using defaults");
        return saveConfig(); // Create default config file
    }

    File file = LITTLEFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return false;
    }

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to parse config file");
        return false;
    }

    // Load PID config
    pidConfig.kp = doc["pid"]["kp"] | DEFAULT_KP;
    pidConfig.ki = doc["pid"]["ki"] | DEFAULT_KI;
    pidConfig.kd = doc["pid"]["kd"] | DEFAULT_KD;
    pidConfig.setpoint = doc["pid"]["setpoint"] | DEFAULT_SETPOINT;
    pidConfig.autoMode = doc["pid"]["autoMode"] | true;

    // Load WiFi config
    strlcpy(wifiConfig.ssid, doc["wifi"]["ssid"] | WIFI_SSID, sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.password, doc["wifi"]["password"] | WIFI_PASSWORD, sizeof(wifiConfig.password));
    strlcpy(wifiConfig.hostname, doc["wifi"]["hostname"] | HOSTNAME, sizeof(wifiConfig.hostname));

    Serial.println("Config loaded successfully");
    return true;
}

bool ConfigManager::saveConfig() {
    DynamicJsonDocument doc(1024);

    // Save PID config
    doc["pid"]["kp"] = pidConfig.kp;
    doc["pid"]["ki"] = pidConfig.ki;
    doc["pid"]["kd"] = pidConfig.kd;
    doc["pid"]["setpoint"] = pidConfig.setpoint;
    doc["pid"]["autoMode"] = pidConfig.autoMode;

    // Save WiFi config
    doc["wifi"]["ssid"] = wifiConfig.ssid;
    doc["wifi"]["password"] = wifiConfig.password;
    doc["wifi"]["hostname"] = wifiConfig.hostname;

    File file = LITTLEFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("Failed to create config file");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write config file");
        file.close();
        return false;
    }

    file.close();
    Serial.println("Config saved successfully");
    return true;
}

void ConfigManager::setPIDConfig(const PIDConfig& config) {
    pidConfig = config;
    saveConfig();
}

void ConfigManager::setWiFiConfig(const WiFiConfig& config) {
    wifiConfig = config;
    saveConfig();
}

void ConfigManager::setPIDTunings(double kp, double ki, double kd) {
    pidConfig.kp = kp;
    pidConfig.ki = ki;
    pidConfig.kd = kd;
    saveConfig();
}

void ConfigManager::setSetpoint(double setpoint) {
    pidConfig.setpoint = setpoint;
    saveConfig();
}

void ConfigManager::setAutoMode(bool mode) {
    pidConfig.autoMode = mode;
    saveConfig();
}
