#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "config.h"

#ifdef USE_TFT_DISPLAY
    #include <TFT_eSPI.h>
    #include <SPI.h>
#endif

#ifdef USE_OLED_DISPLAY
    #include <U8g2lib.h>
    #include <Wire.h>
#endif

enum DisplayPage {
    PAGE_MAIN,
    PAGE_SETTINGS,
    PAGE_TUNING,
    PAGE_SENSORS,
    PAGE_NETWORK
};

class DisplayManager {
private:
    #ifdef USE_TFT_DISPLAY
        TFT_eSPI tft;
    #endif

    #ifdef USE_OLED_DISPLAY
        U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
    #endif

    DisplayPage currentPage;
    unsigned long lastUpdate;
    static const unsigned int UPDATE_INTERVAL = 200; // ms

public:
    DisplayManager();
    void begin();
    void update();

    void showMainScreen(double input, double output, double setpoint, bool pidAuto);
    void showSettingsScreen(int selectedItem);
    void showTuningScreen(double kp, double ki, double kd);
    void showSensorsScreen(float temp, float humidity, int position);
    void showNetworkScreen(const char* ip, bool wifiConnected);

    void setPage(DisplayPage page);
    DisplayPage getCurrentPage() const { return currentPage; }

    void clear();
    void drawMessage(const char* message);

private:
    void drawHeader(const char* title);
    void drawProgressBar(int x, int y, int width, int height, float percentage);
};

#endif // DISPLAY_MANAGER_H
