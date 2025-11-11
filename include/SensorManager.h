#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

enum SensorType {
    SENSOR_DHT22,
    SENSOR_BME280,
    SENSOR_ENCODER,
    SENSOR_NONE
};

class SensorManager {
private:
    DHT dht;
    Adafruit_BME280 bme;

    float temperature;
    float humidity;
    float pressure;
    int encoderPosition;
    volatile int encoderCount;

    SensorType activeSensor;
    unsigned long lastReadTime;
    static const unsigned int READ_INTERVAL = 2000; // ms

    bool dhtAvailable;
    bool bmeAvailable;

    static SensorManager* instance;
    static void IRAM_ATTR handleEncoderISR();

public:
    SensorManager();
    void begin();
    void update();

    float getTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    float getPressure() const { return pressure; }
    int getEncoderPosition() const { return encoderPosition; }

    void resetEncoder();
    bool isDHTAvailable() const { return dhtAvailable; }
    bool isBMEAvailable() const { return bmeAvailable; }

    SensorType getActiveSensor() const { return activeSensor; }
    void setActiveSensor(SensorType type) { activeSensor = type; }

private:
    void readDHT();
    void readBME();
    void readEncoder();
};

#endif // SENSOR_MANAGER_H
