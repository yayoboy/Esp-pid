#include "ConfigManager.h"

ConfigManager::ConfigManager() {
    // Set PID defaults
    pidConfig.kp = DEFAULT_KP;
    pidConfig.ki = DEFAULT_KI;
    pidConfig.kd = DEFAULT_KD;
    pidConfig.setpoint = DEFAULT_SETPOINT;
    pidConfig.autoMode = true;

    // Set WiFi defaults (empty - will trigger captive portal)
    wifiConfig.ssid[0] = '\0';  // Empty SSID to trigger captive portal on first boot
    wifiConfig.password[0] = '\0';
    strncpy(wifiConfig.hostname, HOSTNAME, sizeof(wifiConfig.hostname));

    // Set Pin defaults from config.h
    pinConfig.dht_pin = DHT_PIN;
    pinConfig.bme_sda = BME_SDA;
    pinConfig.bme_scl = BME_SCL;
    pinConfig.encoder_a = ENCODER_PIN_A;
    pinConfig.encoder_b = ENCODER_PIN_B;
    pinConfig.thermo_cs = THERMO_CS;
    pinConfig.thermo_sck = THERMO_SCK;
    pinConfig.thermo_miso = THERMO_MISO;
    pinConfig.ds18b20_pin = DS18B20_PIN;

    pinConfig.pwm_output = PID_OUTPUT_PIN;
    pinConfig.relay_1 = RELAY_PIN_1;
    pinConfig.relay_2 = RELAY_PIN_2;
    pinConfig.relay_3 = RELAY_PIN_3;
    pinConfig.relay_4 = RELAY_PIN_4;

    #ifdef USE_TFT_DISPLAY
    pinConfig.tft_cs = TFT_CS;
    pinConfig.tft_dc = TFT_DC;
    pinConfig.tft_rst = TFT_RST;
    pinConfig.tft_mosi = TFT_MOSI;
    pinConfig.tft_sclk = TFT_SCLK;
    pinConfig.tft_miso = TFT_MISO;
    #endif

    #ifdef USE_OLED_DISPLAY
    pinConfig.oled_sda = OLED_SDA;
    pinConfig.oled_scl = OLED_SCL;
    #endif
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

    DynamicJsonDocument doc(2048); // Increased size for pin config
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

    // Load WiFi config (empty defaults will trigger captive portal)
    strlcpy(wifiConfig.ssid, doc["wifi"]["ssid"] | "", sizeof(wifiConfig.ssid));
    strlcpy(wifiConfig.password, doc["wifi"]["password"] | "", sizeof(wifiConfig.password));
    strlcpy(wifiConfig.hostname, doc["wifi"]["hostname"] | HOSTNAME, sizeof(wifiConfig.hostname));

    // Load Pin config
    if (doc.containsKey("pins")) {
        JsonObject pins = doc["pins"];

        // Sensor pins
        pinConfig.dht_pin = pins["dht"] | DHT_PIN;
        pinConfig.bme_sda = pins["bme_sda"] | BME_SDA;
        pinConfig.bme_scl = pins["bme_scl"] | BME_SCL;
        pinConfig.encoder_a = pins["encoder_a"] | ENCODER_PIN_A;
        pinConfig.encoder_b = pins["encoder_b"] | ENCODER_PIN_B;
        pinConfig.thermo_cs = pins["thermo_cs"] | THERMO_CS;
        pinConfig.thermo_sck = pins["thermo_sck"] | THERMO_SCK;
        pinConfig.thermo_miso = pins["thermo_miso"] | THERMO_MISO;
        pinConfig.ds18b20_pin = pins["ds18b20"] | DS18B20_PIN;

        // Output pins
        pinConfig.pwm_output = pins["pwm"] | PID_OUTPUT_PIN;
        pinConfig.relay_1 = pins["relay_1"] | RELAY_PIN_1;
        pinConfig.relay_2 = pins["relay_2"] | RELAY_PIN_2;
        pinConfig.relay_3 = pins["relay_3"] | RELAY_PIN_3;
        pinConfig.relay_4 = pins["relay_4"] | RELAY_PIN_4;

        // Display pins
        #ifdef USE_TFT_DISPLAY
        pinConfig.tft_cs = pins["tft_cs"] | TFT_CS;
        pinConfig.tft_dc = pins["tft_dc"] | TFT_DC;
        pinConfig.tft_rst = pins["tft_rst"] | TFT_RST;
        pinConfig.tft_mosi = pins["tft_mosi"] | TFT_MOSI;
        pinConfig.tft_sclk = pins["tft_sclk"] | TFT_SCLK;
        pinConfig.tft_miso = pins["tft_miso"] | TFT_MISO;
        #endif

        #ifdef USE_OLED_DISPLAY
        pinConfig.oled_sda = pins["oled_sda"] | OLED_SDA;
        pinConfig.oled_scl = pins["oled_scl"] | OLED_SCL;
        #endif

        Serial.println("Pin configuration loaded");
    }

    Serial.println("Config loaded successfully");
    return true;
}

bool ConfigManager::saveConfig() {
    DynamicJsonDocument doc(2048); // Increased size for pin config

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

    // Save Pin config
    JsonObject pins = doc.createNestedObject("pins");

    // Sensor pins
    pins["dht"] = pinConfig.dht_pin;
    pins["bme_sda"] = pinConfig.bme_sda;
    pins["bme_scl"] = pinConfig.bme_scl;
    pins["encoder_a"] = pinConfig.encoder_a;
    pins["encoder_b"] = pinConfig.encoder_b;
    pins["thermo_cs"] = pinConfig.thermo_cs;
    pins["thermo_sck"] = pinConfig.thermo_sck;
    pins["thermo_miso"] = pinConfig.thermo_miso;
    pins["ds18b20"] = pinConfig.ds18b20_pin;

    // Output pins
    pins["pwm"] = pinConfig.pwm_output;
    pins["relay_1"] = pinConfig.relay_1;
    pins["relay_2"] = pinConfig.relay_2;
    pins["relay_3"] = pinConfig.relay_3;
    pins["relay_4"] = pinConfig.relay_4;

    // Display pins
    pins["tft_cs"] = pinConfig.tft_cs;
    pins["tft_dc"] = pinConfig.tft_dc;
    pins["tft_rst"] = pinConfig.tft_rst;
    pins["tft_mosi"] = pinConfig.tft_mosi;
    pins["tft_sclk"] = pinConfig.tft_sclk;
    pins["tft_miso"] = pinConfig.tft_miso;
    pins["oled_sda"] = pinConfig.oled_sda;
    pins["oled_scl"] = pinConfig.oled_scl;

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

void ConfigManager::setPinConfig(const PinConfig& config) {
    pinConfig = config;
    saveConfig();
    Serial.println("Pin configuration updated - RESTART REQUIRED");
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

bool ConfigManager::validatePin(uint8_t pin, bool allowInputOnly) {
    // ESP32 pin validation

    // Invalid pins (do not exist or reserved)
    if (pin > 39) return false;

    // Flash pins (6-11) - avoid
    if (pin >= 6 && pin <= 11) return false;

    // Input-only pins (34-39)
    if (pin >= 34 && pin <= 39) {
        return allowInputOnly;
    }

    // Valid GPIO pins
    return true;
}

void ConfigManager::setSensorPin(const char* sensor, uint8_t pin) {
    bool inputOnly = false;

    if (strcmp(sensor, "dht") == 0) {
        if (validatePin(pin)) {
            pinConfig.dht_pin = pin;
            Serial.printf("DHT pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(sensor, "bme_sda") == 0) {
        if (validatePin(pin)) {
            pinConfig.bme_sda = pin;
            Serial.printf("BME SDA pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(sensor, "bme_scl") == 0) {
        if (validatePin(pin)) {
            pinConfig.bme_scl = pin;
            Serial.printf("BME SCL pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(sensor, "encoder_a") == 0) {
        if (validatePin(pin, true)) { // Encoder can use input-only
            pinConfig.encoder_a = pin;
            Serial.printf("Encoder A pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(sensor, "encoder_b") == 0) {
        if (validatePin(pin, true)) {
            pinConfig.encoder_b = pin;
            Serial.printf("Encoder B pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(sensor, "thermo_cs") == 0) {
        if (validatePin(pin)) {
            pinConfig.thermo_cs = pin;
            Serial.printf("Thermocouple CS pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(sensor, "ds18b20") == 0) {
        if (validatePin(pin)) {
            pinConfig.ds18b20_pin = pin;
            Serial.printf("DS18B20 pin set to GPIO %d\n", pin);
        }
    }

    saveConfig();
}

void ConfigManager::setOutputPin(const char* output, uint8_t pin) {
    if (strcmp(output, "pwm") == 0) {
        if (validatePin(pin)) {
            pinConfig.pwm_output = pin;
            Serial.printf("PWM output pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(output, "relay_1") == 0) {
        if (validatePin(pin)) {
            pinConfig.relay_1 = pin;
            Serial.printf("Relay 1 pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(output, "relay_2") == 0) {
        if (validatePin(pin)) {
            pinConfig.relay_2 = pin;
            Serial.printf("Relay 2 pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(output, "relay_3") == 0) {
        if (validatePin(pin)) {
            pinConfig.relay_3 = pin;
            Serial.printf("Relay 3 pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(output, "relay_4") == 0) {
        if (validatePin(pin)) {
            pinConfig.relay_4 = pin;
            Serial.printf("Relay 4 pin set to GPIO %d\n", pin);
        }
    }

    saveConfig();
}

void ConfigManager::setDisplayPin(const char* display, uint8_t pin) {
    if (strcmp(display, "tft_cs") == 0) {
        if (validatePin(pin)) {
            pinConfig.tft_cs = pin;
            Serial.printf("TFT CS pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(display, "tft_dc") == 0) {
        if (validatePin(pin)) {
            pinConfig.tft_dc = pin;
            Serial.printf("TFT DC pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(display, "oled_sda") == 0) {
        if (validatePin(pin)) {
            pinConfig.oled_sda = pin;
            Serial.printf("OLED SDA pin set to GPIO %d\n", pin);
        }
    }
    else if (strcmp(display, "oled_scl") == 0) {
        if (validatePin(pin)) {
            pinConfig.oled_scl = pin;
            Serial.printf("OLED SCL pin set to GPIO %d\n", pin);
        }
    }

    saveConfig();
}
