#ifndef CONFIG_RP2040_H
#define CONFIG_RP2040_H

#include <Arduino.h>

// Device Configuration
#define DEVICE_NAME "RP2040-PID"
#define FIRMWARE_VERSION "1.0.0"

// Display Selection (uncomment one)
#define USE_TFT_DISPLAY     // For ILI9341/ST7789 240x320
// #define USE_OLED_DISPLAY // For SSD1315 0.96" OLED

// Pin Definitions - TFT Display (ILI9341/ST7789)
#ifdef USE_TFT_DISPLAY
    #define TFT_CS    17
    #define TFT_DC    16
    #define TFT_RST   15
    #define TFT_MOSI  19  // SPI1 TX
    #define TFT_SCLK  18  // SPI1 SCK
    #define TFT_MISO  20  // SPI1 RX
    #define TOUCH_CS  14
    #define TOUCH_IRQ 13
#endif

// Pin Definitions - OLED Display (SSD1315)
#ifdef USE_OLED_DISPLAY
    #define OLED_SDA  4   // I2C0 SDA
    #define OLED_SCL  5   // I2C0 SCL
    #define OLED_ADDR 0x3C
#endif

// Input Pins (Encoder + Buttons)
#define ENCODER_PIN_A  6   // Rotary encoder A
#define ENCODER_PIN_B  7   // Rotary encoder B
#define ENCODER_BTN    8   // Encoder button (enter/select)
#define BTN_BACK       9   // Back button
#define BTN_UP         10  // Up button (optional)
#define BTN_DOWN       11  // Down button (optional)

// Sensor Pins
#define DHT_PIN        2   // DHT22 Temperature/Humidity
#define DHT_TYPE       DHT22
#define BME_SDA        4   // BME280 I2C (shared with OLED if used)
#define BME_SCL        5
#define ENCODER_POS_A  12  // Position encoder A (for control variable)
#define ENCODER_POS_B  13  // Position encoder B

// Thermocouple Pins (MAX6675/MAX31855) - SPI0
#define THERMO_CS      1   // Chip Select
#define THERMO_SCK     2   // SPI0 SCK (shared)
#define THERMO_MISO    0   // SPI0 RX (shared)

// DS18B20 OneWire Pin
#define DS18B20_PIN    3   // OneWire bus for DS18B20

// Output Control Pins
#define PID_OUTPUT_PIN 21  // PWM output for control
#define RELAY_PIN_1    22  // Relay 1 (Heating)
#define RELAY_PIN_2    26  // Relay 2 (Cooling/Fan)
#define RELAY_PIN_3    27  // Relay 3 (Alarm/Aux)
#define RELAY_PIN_4    28  // Relay 4 (Spare)

// Status LED
#define LED_STATUS     25  // Built-in LED on Pico

// PID Default Values
#define DEFAULT_KP     2.0
#define DEFAULT_KI     5.0
#define DEFAULT_KD     1.0
#define DEFAULT_SETPOINT 25.0
#define PID_SAMPLE_TIME  100  // milliseconds

// Output Mode
#define OUTPUT_MODE_PWM    0
#define OUTPUT_MODE_RELAY  1
#define OUTPUT_MODE_DUAL   2

// Relay Control
#define RELAY_HYSTERESIS 0.5
#define RELAY_MIN_CYCLE  1000

// Menu Configuration
#define MENU_TIMEOUT     30000  // Return to main after 30s inactivity
#define DEBOUNCE_DELAY   50     // Button debounce ms
#define ENCODER_SENSITIVITY 4   // Pulses per detent

// EEPROM Configuration
#define EEPROM_SIZE      1024
#define EEPROM_MAGIC     0xCAFE  // Magic number to validate config
#define CONFIG_ADDRESS   0

#endif // CONFIG_RP2040_H
