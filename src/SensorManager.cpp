#include "SensorManager.h"

SensorManager* SensorManager::instance = nullptr;

SensorManager::SensorManager()
    : dht(DHT_PIN, DHT_TYPE),
      max31855(THERMO_CS, THERMO_MISO, THERMO_SCK),
      max6675(THERMO_SCK, THERMO_CS, THERMO_MISO),
      oneWire(DS18B20_PIN),
      ds18b20(&oneWire),
      temperature(0), humidity(0), pressure(0),
      encoderPosition(0), encoderCount(0),
      dhtAvailable(false), bmeAvailable(false),
      max31855Available(false), max6675Available(false),
      ds18b20Available(false), ds18b20Count(0),
      activeSensor(SENSOR_NONE),
      autoCalibrating(false), autoCalSampleCount(0), autoCalReference(0),
      lastReadTime(0) {

    instance = this;

    // Initialize calibration
    calibration.offset = 0.0;
    calibration.scale = 1.0;
    calibration.enabled = false;
}

void SensorManager::begin() {
    Serial.println("\n========================================");
    Serial.println("Initializing sensors...");
    Serial.println("========================================");

    detectSensors();

    // Initialize Rotary Encoder
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), handleEncoderISR, CHANGE);
    Serial.println("[✓] Encoder initialized");

    Serial.println("========================================");
    Serial.printf("Active Sensor: %s\n", getSensorName().c_str());
    Serial.println("========================================\n");

    // First read
    update();
}

void SensorManager::detectSensors() {
    // Try DHT22
    dht.begin();
    delay(100);
    float testTemp = dht.readTemperature();
    if (!isnan(testTemp) && testTemp > -40 && testTemp < 80) {
        dhtAvailable = true;
        if (activeSensor == SENSOR_NONE) activeSensor = SENSOR_DHT22;
        Serial.println("[✓] DHT22 sensor detected");
    } else {
        Serial.println("[✗] DHT22 sensor not found");
    }

    // Try BME280
    Wire.begin(BME_SDA, BME_SCL);
    if (bme.begin(0x76) || bme.begin(0x77)) {
        bmeAvailable = true;
        if (activeSensor == SENSOR_NONE) activeSensor = SENSOR_BME280;
        Serial.println("[✓] BME280 sensor detected");
    } else {
        Serial.println("[✗] BME280 sensor not found");
    }

    // Try MAX31855
    delay(100);
    double tempMAX31855 = max31855.readCelsius();
    if (!isnan(tempMAX31855) && tempMAX31855 > -100 && tempMAX31855 < 1500) {
        max31855Available = true;
        if (activeSensor == SENSOR_NONE) activeSensor = SENSOR_MAX31855;
        Serial.println("[✓] MAX31855 thermocouple detected");
    } else {
        Serial.println("[✗] MAX31855 thermocouple not found");
    }

    // Try MAX6675
    if (!max31855Available) {
        delay(500); // MAX6675 needs longer delay
        double tempMAX6675 = max6675.readCelsius();
        if (tempMAX6675 > 0 && tempMAX6675 < 1024) {
            max6675Available = true;
            if (activeSensor == SENSOR_NONE) activeSensor = SENSOR_MAX6675;
            Serial.println("[✓] MAX6675 thermocouple detected");
        } else {
            Serial.println("[✗] MAX6675 thermocouple not found");
        }
    }

    // Try DS18B20
    ds18b20.begin();
    ds18b20Count = ds18b20.getDeviceCount();
    if (ds18b20Count > 0) {
        ds18b20Available = true;
        if (activeSensor == SENSOR_NONE) activeSensor = SENSOR_DS18B20;

        // Get first device address
        if (ds18b20.getAddress(ds18b20Address, 0)) {
            ds18b20.setResolution(ds18b20Address, 12); // Max resolution
        }

        Serial.printf("[✓] DS18B20 sensor(s) detected: %d\n", ds18b20Count);
    } else {
        Serial.println("[✗] DS18B20 sensor not found");
    }

    if (activeSensor == SENSOR_NONE) {
        Serial.println("[!] WARNING: No temperature sensors detected!");
        Serial.println("[!] System will use encoder as input");
        activeSensor = SENSOR_ENCODER;
    }
}

void SensorManager::update() {
    unsigned long now = millis();
    if (now - lastReadTime < READ_INTERVAL) return;

    lastReadTime = now;

    // Read active sensor
    switch (activeSensor) {
        case SENSOR_DHT22:
            if (dhtAvailable) readDHT();
            break;
        case SENSOR_BME280:
            if (bmeAvailable) readBME();
            break;
        case SENSOR_MAX31855:
            if (max31855Available) readMAX31855();
            break;
        case SENSOR_MAX6675:
            if (max6675Available) readMAX6675();
            break;
        case SENSOR_DS18B20:
            if (ds18b20Available) readDS18B20();
            break;
        case SENSOR_ENCODER:
            readEncoder();
            break;
        default:
            break;
    }

    // Always read encoder for position
    readEncoder();

    // Update auto-calibration if active
    if (autoCalibrating) {
        updateAutoCalibration();
    }
}

void SensorManager::readDHT() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        temperature = t;
        humidity = h;
    } else {
        Serial.println("[!] Failed to read from DHT sensor");
    }
}

void SensorManager::readBME() {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F; // Convert to hPa
}

void SensorManager::readMAX31855() {
    double temp = max31855.readCelsius();

    if (isnan(temp)) {
        Serial.println("[!] MAX31855 read error");

        // Check for specific errors
        uint8_t fault = max31855.readError();
        if (fault) {
            if (fault & MAX31855_FAULT_OPEN)
                Serial.println("    - Thermocouple open circuit!");
            if (fault & MAX31855_FAULT_SHORT_GND)
                Serial.println("    - Thermocouple short to GND!");
            if (fault & MAX31855_FAULT_SHORT_VCC)
                Serial.println("    - Thermocouple short to VCC!");
        }
    } else {
        temperature = temp;
    }
}

void SensorManager::readMAX6675() {
    double temp = max6675.readCelsius();

    if (temp >= 0 && temp < 1024) {
        temperature = temp;
    } else {
        Serial.println("[!] MAX6675 read error");
    }
}

void SensorManager::readDS18B20() {
    ds18b20.requestTemperatures();

    float temp = ds18b20.getTempCByIndex(0);

    if (temp != DEVICE_DISCONNECTED_C && temp > -55 && temp < 125) {
        temperature = temp;
    } else {
        Serial.println("[!] DS18B20 read error");
    }
}

void SensorManager::readEncoder() {
    encoderPosition = encoderCount;
}

void SensorManager::resetEncoder() {
    encoderCount = 0;
    encoderPosition = 0;
}

void SensorManager::setActiveSensor(SensorType type) {
    // Check if sensor is available
    switch (type) {
        case SENSOR_DHT22:
            if (!dhtAvailable) return;
            break;
        case SENSOR_BME280:
            if (!bmeAvailable) return;
            break;
        case SENSOR_MAX31855:
            if (!max31855Available) return;
            break;
        case SENSOR_MAX6675:
            if (!max6675Available) return;
            break;
        case SENSOR_DS18B20:
            if (!ds18b20Available) return;
            break;
        default:
            break;
    }

    activeSensor = type;
    Serial.printf("Active sensor changed to: %s\n", getSensorName().c_str());
}

String SensorManager::getSensorName() const {
    return getSensorName(activeSensor);
}

String SensorManager::getSensorName(SensorType type) const {
    switch (type) {
        case SENSOR_DHT22:    return "DHT22";
        case SENSOR_BME280:   return "BME280";
        case SENSOR_MAX31855: return "MAX31855 K-Type";
        case SENSOR_MAX6675:  return "MAX6675 K-Type";
        case SENSOR_DS18B20:  return "DS18B20";
        case SENSOR_ENCODER:  return "Encoder";
        default:              return "None";
    }
}

// Calibration functions
void SensorManager::setCalibration(float offset, float scale) {
    calibration.offset = offset;
    calibration.scale = scale;
    Serial.printf("Calibration set: offset=%.2f, scale=%.4f\n", offset, scale);
}

void SensorManager::getCalibration(float& offset, float& scale) const {
    offset = calibration.offset;
    scale = calibration.scale;
}

void SensorManager::enableCalibration(bool enable) {
    calibration.enabled = enable;
    Serial.printf("Calibration %s\n", enable ? "enabled" : "disabled");
}

float SensorManager::applyCalibration(float value) const {
    if (!calibration.enabled) return value;
    return (value * calibration.scale) + calibration.offset;
}

// Auto-calibration functions
void SensorManager::startAutoCalibration(float referenceValue) {
    autoCalibrating = true;
    autoCalSampleCount = 0;
    autoCalReference = referenceValue;

    Serial.println("\n========================================");
    Serial.println("Starting auto-calibration");
    Serial.printf("Reference value: %.2f\n", referenceValue);
    Serial.println("Please wait for 100 samples...");
    Serial.println("========================================\n");
}

void SensorManager::stopAutoCalibration() {
    if (!autoCalibrating) return;

    autoCalibrating = false;

    if (autoCalSampleCount > 0) {
        // Calculate average
        float sum = 0;
        for (int i = 0; i < autoCalSampleCount; i++) {
            sum += autoCalSamples[i];
        }
        float average = sum / autoCalSampleCount;

        // Calculate offset
        float offset = autoCalReference - average;

        // Apply calibration
        calibration.offset = offset;
        calibration.scale = 1.0;
        calibration.enabled = true;

        Serial.println("\n========================================");
        Serial.println("Auto-calibration complete!");
        Serial.printf("Samples collected: %d\n", autoCalSampleCount);
        Serial.printf("Average reading: %.2f\n", average);
        Serial.printf("Reference value: %.2f\n", autoCalReference);
        Serial.printf("Calculated offset: %.2f\n", offset);
        Serial.println("========================================\n");
    }

    autoCalSampleCount = 0;
}

bool SensorManager::isAutoCalibrating() const {
    return autoCalibrating;
}

float SensorManager::getAutoCalProgress() const {
    if (!autoCalibrating) return 0;
    return (float)autoCalSampleCount / 100.0 * 100.0;
}

void SensorManager::updateAutoCalibration() {
    if (autoCalSampleCount < 100) {
        autoCalSamples[autoCalSampleCount++] = temperature;

        if (autoCalSampleCount % 10 == 0) {
            Serial.printf("Auto-calibration progress: %d%%\n", autoCalSampleCount);
        }
    } else {
        stopAutoCalibration();
    }
}

void IRAM_ATTR SensorManager::handleEncoderISR() {
    if (instance) {
        int stateA = digitalRead(ENCODER_PIN_A);
        int stateB = digitalRead(ENCODER_PIN_B);

        if (stateA == stateB) {
            instance->encoderCount++;
        } else {
            instance->encoderCount--;
        }
    }
}
