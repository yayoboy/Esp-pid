#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "PIDController.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "WebServer.h"
#include "OTAManager.h"
#include "ConfigManager.h"

// Global objects
PIDController pid;
SensorManager sensors;
DisplayManager display;
WebServerManager webServer;
OTAManager ota;
ConfigManager configManager;

// Variables
double currentInput = 0.0;
double currentOutput = 0.0;
unsigned long lastPIDUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastWebUpdate = 0;

const unsigned int PID_UPDATE_INTERVAL = 100;   // 100ms
const unsigned int DISPLAY_UPDATE_INTERVAL = 500; // 500ms
const unsigned int WEB_UPDATE_INTERVAL = 1000;   // 1s

// Function declarations
void setupWiFi();
void updatePID();
void updateDisplay();
void updateWebClients();

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println("ESP32 PID Controller");
    Serial.println("========================================\n");

    // Initialize Configuration Manager
    Serial.println("Initializing configuration...");
    if (!configManager.begin()) {
        Serial.println("Failed to initialize config, using defaults");
    }

    // Load configuration
    PIDConfig pidConfig = configManager.getPIDConfig();

    // Initialize Display
    Serial.println("Initializing display...");
    display.begin();
    display.drawMessage("Initializing...");

    // Initialize Sensors
    Serial.println("Initializing sensors...");
    sensors.begin();

    // Initialize PID Controller
    Serial.println("Initializing PID controller...");
    pid.begin(pidConfig.kp, pidConfig.ki, pidConfig.kd, pidConfig.setpoint);
    pid.setOutputLimits(0, 255);
    pid.setMode(pidConfig.autoMode);

    // Setup PWM output
    ledcSetup(0, 5000, 8); // Channel 0, 5kHz, 8-bit resolution
    ledcAttachPin(PID_OUTPUT_PIN, 0);

    // Initialize WiFi
    display.drawMessage("Connecting WiFi...");
    setupWiFi();

    // Initialize OTA
    Serial.println("Initializing OTA...");
    ota.begin();

    // Initialize Web Server
    Serial.println("Initializing web server...");
    webServer.begin(&pid, &sensors, &currentInput, &currentOutput);

    // Show main screen
    display.setPage(PAGE_MAIN);

    Serial.println("\n========================================");
    Serial.println("System Ready!");
    Serial.println("========================================");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Web interface: http://" + WiFi.localIP().toString());
    Serial.println("OTA hostname: " + String(HOSTNAME));
    Serial.println("========================================\n");

    lastPIDUpdate = millis();
    lastDisplayUpdate = millis();
    lastWebUpdate = millis();
}

void loop() {
    unsigned long now = millis();

    // Handle OTA updates
    ota.handle();

    // Update sensors periodically
    sensors.update();

    // Get current input from active sensor
    if (sensors.isDHTAvailable() || sensors.isBMEAvailable()) {
        currentInput = sensors.getTemperature();
    } else {
        // Use encoder position as input if no temperature sensor
        currentInput = sensors.getEncoderPosition();
    }

    // Update PID
    if (now - lastPIDUpdate >= PID_UPDATE_INTERVAL) {
        updatePID();
        lastPIDUpdate = now;
    }

    // Update Display
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = now;
    }

    // Update Web Clients
    if (now - lastWebUpdate >= WEB_UPDATE_INTERVAL) {
        updateWebClients();
        lastWebUpdate = now;
    }

    delay(10); // Small delay to prevent watchdog issues
}

void setupWiFi() {
    WiFiConfig wifiConfig = configManager.getWiFiConfig();

    // Try to connect to WiFi
    WiFi.mode(WIFI_AP_STA);
    WiFi.setHostname(wifiConfig.hostname);

    Serial.printf("Connecting to WiFi: %s\n", wifiConfig.ssid);
    WiFi.begin(wifiConfig.ssid, wifiConfig.password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect to WiFi, starting AP mode");
        WiFi.softAP(wifiConfig.ssid, wifiConfig.password);
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    }
}

void updatePID() {
    if (pid.isAuto()) {
        currentOutput = pid.compute(currentInput);

        // Apply output to PWM
        ledcWrite(0, (int)currentOutput);
    }
}

void updateDisplay() {
    switch (display.getCurrentPage()) {
        case PAGE_MAIN:
            display.showMainScreen(currentInput, currentOutput, pid.getSetpoint(), pid.isAuto());
            break;

        case PAGE_TUNING:
            display.showTuningScreen(pid.getKp(), pid.getKi(), pid.getKd());
            break;

        case PAGE_SENSORS:
            display.showSensorsScreen(
                sensors.getTemperature(),
                sensors.getHumidity(),
                sensors.getEncoderPosition()
            );
            break;

        case PAGE_NETWORK:
            {
                String ip = WiFi.localIP().toString();
                if (WiFi.status() != WL_CONNECTED) {
                    ip = WiFi.softAPIP().toString();
                }
                display.showNetworkScreen(ip.c_str(), WiFi.status() == WL_CONNECTED);
            }
            break;

        default:
            break;
    }
}

void updateWebClients() {
    webServer.sendUpdate();
}
