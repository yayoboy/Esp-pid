#include "SensorManager.h"

SensorManager* SensorManager::instance = nullptr;

SensorManager::SensorManager()
    : dht(DHT_PIN, DHT_TYPE),
      temperature(0), humidity(0), pressure(0),
      encoderPosition(0), encoderCount(0),
      activeSensor(SENSOR_NONE),
      lastReadTime(0),
      dhtAvailable(false),
      bmeAvailable(false) {
    instance = this;
}

void SensorManager::begin() {
    Serial.println("Initializing sensors...");

    // Initialize DHT22
    dht.begin();
    delay(100);
    float testTemp = dht.readTemperature();
    if (!isnan(testTemp)) {
        dhtAvailable = true;
        activeSensor = SENSOR_DHT22;
        Serial.println("DHT22 sensor detected");
    } else {
        Serial.println("DHT22 sensor not found");
    }

    // Initialize BME280
    Wire.begin(BME_SDA, BME_SCL);
    if (bme.begin(0x76)) { // Try address 0x76
        bmeAvailable = true;
        activeSensor = SENSOR_BME280;
        Serial.println("BME280 sensor detected");
    } else if (bme.begin(0x77)) { // Try address 0x77
        bmeAvailable = true;
        activeSensor = SENSOR_BME280;
        Serial.println("BME280 sensor detected at 0x77");
    } else {
        Serial.println("BME280 sensor not found");
    }

    // Initialize Rotary Encoder
    pinMode(ENCODER_PIN_A, INPUT_PULLUP);
    pinMode(ENCODER_PIN_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), handleEncoderISR, CHANGE);
    Serial.println("Encoder initialized");

    // First read
    update();
}

void SensorManager::update() {
    unsigned long now = millis();
    if (now - lastReadTime < READ_INTERVAL) return;

    lastReadTime = now;

    if (dhtAvailable && activeSensor == SENSOR_DHT22) {
        readDHT();
    }

    if (bmeAvailable && activeSensor == SENSOR_BME280) {
        readBME();
    }

    readEncoder();
}

void SensorManager::readDHT() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
        temperature = t;
        humidity = h;
        Serial.printf("DHT22 - Temp: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
    } else {
        Serial.println("Failed to read from DHT sensor");
    }
}

void SensorManager::readBME() {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F; // Convert to hPa

    Serial.printf("BME280 - Temp: %.1f°C, Humidity: %.1f%%, Pressure: %.1f hPa\n",
                  temperature, humidity, pressure);
}

void SensorManager::readEncoder() {
    encoderPosition = encoderCount;
}

void SensorManager::resetEncoder() {
    encoderCount = 0;
    encoderPosition = 0;
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
