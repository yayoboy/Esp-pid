#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_MAX31855.h>
#include <max6675.h>
#include <OneWire.h>
#include <DallasTemperature.h>

enum SensorType {
    SENSOR_NONE,
    SENSOR_DHT22,
    SENSOR_BME280,
    SENSOR_MAX31855,
    SENSOR_MAX6675,
    SENSOR_DS18B20,
    SENSOR_ENCODER
};

struct SensorCalibration {
    float offset;
    float scale;
    bool enabled;
};

class SensorManager {
private:
    // Sensor objects
    DHT dht;
    Adafruit_BME280 bme;
    Adafruit_MAX31855 max31855;
    MAX6675 max6675;
    OneWire oneWire;
    DallasTemperature ds18b20;

    // Sensor data
    float temperature;
    float humidity;
    float pressure;
    int encoderPosition;
    volatile int encoderCount;

    // Sensor availability
    bool dhtAvailable;
    bool bmeAvailable;
    bool max31855Available;
    bool max6675Available;
    bool ds18b20Available;
    uint8_t ds18b20Count;
    DeviceAddress ds18b20Address;

    // Active sensor
    SensorType activeSensor;

    // Calibration
    SensorCalibration calibration;

    // Auto-calibration
    bool autoCalibrating;
    float autoCalSamples[100];
    int autoCalSampleCount;
    float autoCalReference;

    // Timing
    unsigned long lastReadTime;
    static const unsigned int READ_INTERVAL = 1000; // ms

    static SensorManager* instance;
    static void IRAM_ATTR handleEncoderISR();

public:
    SensorManager();
    void begin();
    void update();

    // Getters
    float getTemperature() const { return applyCalibration(temperature); }
    float getRawTemperature() const { return temperature; }
    float getHumidity() const { return humidity; }
    float getPressure() const { return pressure; }
    int getEncoderPosition() const { return encoderPosition; }

    // Sensor availability
    bool isDHTAvailable() const { return dhtAvailable; }
    bool isBMEAvailable() const { return bmeAvailable; }
    bool isMAX31855Available() const { return max31855Available; }
    bool isMAX6675Available() const { return max6675Available; }
    bool isDS18B20Available() const { return ds18b20Available; }
    uint8_t getDS18B20Count() const { return ds18b20Count; }

    // Active sensor
    SensorType getActiveSensor() const { return activeSensor; }
    void setActiveSensor(SensorType type);
    String getSensorName() const;
    String getSensorName(SensorType type) const;

    // Encoder
    void resetEncoder();

    // Calibration
    void setCalibration(float offset, float scale);
    void getCalibration(float& offset, float& scale) const;
    void enableCalibration(bool enable);
    bool isCalibrationEnabled() const { return calibration.enabled; }
    float applyCalibration(float value) const;

    // Auto-calibration
    void startAutoCalibration(float referenceValue);
    void stopAutoCalibration();
    bool isAutoCalibrating() const { return autoCalibrating; }
    float getAutoCalProgress() const;

    // Sensor detection
    void detectSensors();

private:
    void readDHT();
    void readBME();
    void readMAX31855();
    void readMAX6675();
    void readDS18B20();
    void readEncoder();
    void updateAutoCalibration();
};

#endif // SENSOR_MANAGER_H
