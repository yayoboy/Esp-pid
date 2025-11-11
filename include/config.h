#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// WiFi Configuration
#define WIFI_SSID "ESP32-PID-AP"
#define WIFI_PASSWORD "pid12345"
#define HOSTNAME "esp32-pid"

// Display Selection (uncomment one)
#define USE_TFT_DISPLAY     // For ILI9341/ST7789 240x320
// #define USE_OLED_DISPLAY // For SSD1315 0.96" OLED

// Pin Definitions - TFT Display (ILI9341/ST7789)
#ifdef USE_TFT_DISPLAY
    #define TFT_CS    15
    #define TFT_DC    2
    #define TFT_RST   4
    #define TFT_MOSI  23
    #define TFT_SCLK  18
    #define TFT_MISO  19
    #define TOUCH_CS  5
    #define TOUCH_IRQ 17
#endif

// Pin Definitions - OLED Display (SSD1315)
#ifdef USE_OLED_DISPLAY
    #define OLED_SDA  21
    #define OLED_SCL  22
    #define OLED_ADDR 0x3C
    // 4x4 Keypad pins
    #define KEYPAD_ROW_PINS {13, 12, 14, 27}
    #define KEYPAD_COL_PINS {26, 25, 33, 32}
#endif

// Sensor Pins
#define DHT_PIN       16    // DHT22 Temperature/Humidity
#define DHT_TYPE      DHT22
#define BME_SDA       21    // BME280 (I2C)
#define BME_SCL       22
#define ENCODER_PIN_A 34    // Rotary encoder for position
#define ENCODER_PIN_B 35

// Output Control Pin (PWM for PID output)
#define PID_OUTPUT_PIN 25   // PWM output for control

// PID Default Values
#define DEFAULT_KP     2.0
#define DEFAULT_KI     5.0
#define DEFAULT_KD     1.0
#define DEFAULT_SETPOINT 25.0
#define PID_SAMPLE_TIME  100  // milliseconds

// Web Server
#define WEB_SERVER_PORT 80

// File System
#define CONFIG_FILE "/config.json"

#endif // CONFIG_H
