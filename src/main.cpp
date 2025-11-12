#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "PIDController.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "WebServer.h"
#include "OTAManager.h"
#include "ConfigManager.h"
#include "RelayController.h"
#include "WiFiManager.h"

// Global objects
PIDController pid;
SensorManager sensors;
DisplayManager display;
WebServerManager webServer;
OTAManager ota;
ConfigManager configManager;
RelayController relayController;
WiFiManager wifiManager(&configManager);

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

    // Initialize Relay Controller
    Serial.println("Initializing relay controller...");
    relayController.begin();
    relayController.setMode(RELAY_HEATING_ONLY); // Default mode

    // Initialize WiFi with Captive Portal
    Serial.println("Initializing WiFi...");
    display.drawMessage("WiFi Setup...");
    wifiManager.begin();

    // Wait a moment for WiFi to stabilize
    delay(2000);

    // Initialize OTA (only if connected to WiFi)
    if (wifiManager.getStatus() == WIFI_CONNECTED) {
        Serial.println("Initializing OTA...");
        ota.begin();
    }

    // Initialize Web Server
    Serial.println("Initializing web server...");
    webServer.begin(&pid, &sensors, &currentInput, &currentOutput);

    // Show main screen
    display.setPage(PAGE_MAIN);

    Serial.println("\n========================================");
    Serial.println("System Ready!");
    Serial.println("========================================");
    Serial.print("WiFi Status: ");
    Serial.println(wifiManager.getStatusString());
    Serial.print("IP Address: ");
    Serial.println(wifiManager.getIPAddress());

    if (wifiManager.getStatus() == WIFI_CONNECTED) {
        Serial.println("Web interface: http://" + wifiManager.getIPAddress());
        Serial.println("OTA hostname: " + String(HOSTNAME));
    } else if (wifiManager.getStatus() == WIFI_AP_MODE) {
        Serial.println("Captive Portal active!");
        Serial.println("Connect to WiFi: " + String(AP_SSID));
        Serial.println("Password: " + String(AP_PASSWORD));
        Serial.println("Configuration page: http://" + wifiManager.getIPAddress());
    }
    Serial.println("========================================\n");

    lastPIDUpdate = millis();
    lastDisplayUpdate = millis();
    lastWebUpdate = millis();
}

void loop() {
    unsigned long now = millis();

    // Update WiFi Manager (handles captive portal, reconnection, reset button)
    wifiManager.update();

    // Handle OTA updates (only if connected)
    if (wifiManager.getStatus() == WIFI_CONNECTED) {
        ota.handle();
    }

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

// setupWiFi() removed - now using WiFiManager with captive portal

void updatePID() {
    if (pid.isAuto()) {
        currentOutput = pid.compute(currentInput);

        // Apply output to PWM
        ledcWrite(0, (int)currentOutput);

        // Update relay controller
        relayController.update(currentInput, pid.getSetpoint(), currentOutput);
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
                String ip = wifiManager.getIPAddress();
                bool connected = (wifiManager.getStatus() == WIFI_CONNECTED);
                display.showNetworkScreen(ip.c_str(), connected);
            }
            break;

        default:
            break;
    }
}

void updateWebClients() {
    webServer.sendUpdate();
}
