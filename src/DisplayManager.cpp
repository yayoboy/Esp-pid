#include "DisplayManager.h"

DisplayManager::DisplayManager()
    : currentPage(PAGE_MAIN), lastUpdate(0)
#ifdef USE_OLED_DISPLAY
    , u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA)
#endif
{
}

void DisplayManager::begin() {
    #ifdef USE_TFT_DISPLAY
        tft.init();
        tft.setRotation(1); // Landscape
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.begin();
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.sendBuffer();
    #endif

    Serial.println("Display initialized");
}

void DisplayManager::update() {
    unsigned long now = millis();
    if (now - lastUpdate < UPDATE_INTERVAL) return;
    lastUpdate = now;
}

void DisplayManager::showMainScreen(double input, double output, double setpoint, bool pidAuto) {
    #ifdef USE_TFT_DISPLAY
        tft.fillScreen(TFT_BLACK);
        drawHeader("PID Controller");

        // Setpoint
        tft.setCursor(10, 40);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.setTextSize(2);
        tft.print("Setpoint: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print(setpoint, 1);

        // Current Value
        tft.setCursor(10, 70);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print("Current:  ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print(input, 1);

        // Output
        tft.setCursor(10, 100);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print("Output:   ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print(output, 1);
        tft.print(" %");

        // Output bar
        drawProgressBar(10, 130, 300, 20, output / 255.0 * 100.0);

        // Status
        tft.setCursor(10, 160);
        tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
        tft.setTextSize(1);
        tft.print("Mode: ");
        tft.setTextColor(pidAuto ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.print(pidAuto ? "AUTO" : "MANUAL");
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();

        // Title
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 10, "PID Controller");
        u8g2.drawLine(0, 12, 128, 12);

        // Values
        u8g2.setFont(u8g2_font_6x10_tr);

        char buffer[32];
        sprintf(buffer, "SP: %.1f", setpoint);
        u8g2.drawStr(0, 25, buffer);

        sprintf(buffer, "PV: %.1f", input);
        u8g2.drawStr(0, 38, buffer);

        sprintf(buffer, "Out: %.0f%%", output / 255.0 * 100.0);
        u8g2.drawStr(0, 51, buffer);

        // Status
        u8g2.drawStr(0, 64, pidAuto ? "AUTO" : "MANUAL");

        u8g2.sendBuffer();
    #endif
}

void DisplayManager::showSettingsScreen(int selectedItem) {
    #ifdef USE_TFT_DISPLAY
        tft.fillScreen(TFT_BLACK);
        drawHeader("Settings");

        const char* items[] = {"PID Tuning", "Sensors", "Network", "Back"};
        for (int i = 0; i < 4; i++) {
            tft.setCursor(20, 40 + i * 30);
            if (i == selectedItem) {
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            tft.print(items[i]);
        }
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 10, "Settings");

        const char* items[] = {"Tuning", "Sensors", "Network", "Back"};
        for (int i = 0; i < 4; i++) {
            if (i == selectedItem) {
                u8g2.drawBox(0, 15 + i * 12, 128, 12);
                u8g2.setDrawColor(0);
            }
            u8g2.drawStr(2, 25 + i * 12, items[i]);
            u8g2.setDrawColor(1);
        }

        u8g2.sendBuffer();
    #endif
}

void DisplayManager::showTuningScreen(double kp, double ki, double kd) {
    #ifdef USE_TFT_DISPLAY
        tft.fillScreen(TFT_BLACK);
        drawHeader("PID Tuning");

        tft.setTextSize(2);
        tft.setCursor(10, 40);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.print("Kp: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println(kp, 2);

        tft.setCursor(10, 70);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print("Ki: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println(ki, 2);

        tft.setCursor(10, 100);
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.print("Kd: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println(kd, 2);
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 10, "PID Tuning");

        char buffer[32];
        u8g2.setFont(u8g2_font_6x10_tr);

        sprintf(buffer, "Kp: %.2f", kp);
        u8g2.drawStr(0, 25, buffer);

        sprintf(buffer, "Ki: %.2f", ki);
        u8g2.drawStr(0, 38, buffer);

        sprintf(buffer, "Kd: %.2f", kd);
        u8g2.drawStr(0, 51, buffer);

        u8g2.sendBuffer();
    #endif
}

void DisplayManager::showSensorsScreen(float temp, float humidity, int position) {
    #ifdef USE_TFT_DISPLAY
        tft.fillScreen(TFT_BLACK);
        drawHeader("Sensors");

        tft.setTextSize(2);
        tft.setCursor(10, 40);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print("Temp: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print(temp, 1);
        tft.println(" C");

        tft.setCursor(10, 70);
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
        tft.print("Humidity: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.print(humidity, 1);
        tft.println(" %");

        tft.setCursor(10, 100);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print("Position: ");
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.println(position);
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 10, "Sensors");

        char buffer[32];
        u8g2.setFont(u8g2_font_6x10_tr);

        sprintf(buffer, "T: %.1fC", temp);
        u8g2.drawStr(0, 25, buffer);

        sprintf(buffer, "H: %.1f%%", humidity);
        u8g2.drawStr(0, 38, buffer);

        sprintf(buffer, "Pos: %d", position);
        u8g2.drawStr(0, 51, buffer);

        u8g2.sendBuffer();
    #endif
}

void DisplayManager::showNetworkScreen(const char* ip, bool wifiConnected) {
    #ifdef USE_TFT_DISPLAY
        tft.fillScreen(TFT_BLACK);
        drawHeader("Network");

        tft.setTextSize(2);
        tft.setCursor(10, 40);
        tft.setTextColor(TFT_CYAN, TFT_BLACK);
        tft.print("Status: ");
        tft.setTextColor(wifiConnected ? TFT_GREEN : TFT_RED, TFT_BLACK);
        tft.println(wifiConnected ? "Connected" : "Disconnected");

        if (wifiConnected) {
            tft.setCursor(10, 70);
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
            tft.print("IP: ");
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextSize(1);
            tft.println(ip);
        }
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 10, "Network");

        u8g2.setFont(u8g2_font_6x10_tr);
        u8g2.drawStr(0, 25, wifiConnected ? "Connected" : "Disconnected");

        if (wifiConnected) {
            u8g2.drawStr(0, 38, "IP:");
            u8g2.drawStr(0, 51, ip);
        }

        u8g2.sendBuffer();
    #endif
}

void DisplayManager::setPage(DisplayPage page) {
    currentPage = page;
}

void DisplayManager::clear() {
    #ifdef USE_TFT_DISPLAY
        tft.fillScreen(TFT_BLACK);
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();
        u8g2.sendBuffer();
    #endif
}

void DisplayManager::drawMessage(const char* message) {
    clear();

    #ifdef USE_TFT_DISPLAY
        tft.setCursor(10, 80);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
        tft.println(message);
    #endif

    #ifdef USE_OLED_DISPLAY
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB08_tr);
        u8g2.drawStr(0, 32, message);
        u8g2.sendBuffer();
    #endif
}

void DisplayManager::drawHeader(const char* title) {
    #ifdef USE_TFT_DISPLAY
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.fillRect(0, 0, 320, 30, TFT_BLUE);
        tft.setCursor(10, 8);
        tft.setTextSize(2);
        tft.println(title);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    #endif
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, float percentage) {
    #ifdef USE_TFT_DISPLAY
        tft.drawRect(x, y, width, height, TFT_WHITE);
        int fillWidth = (width - 4) * (percentage / 100.0);
        tft.fillRect(x + 2, y + 2, fillWidth, height - 4, TFT_GREEN);
    #endif
}
