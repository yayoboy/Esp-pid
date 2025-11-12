#include <Arduino.h>
#include "config.h"
#include "PIDController.h"
#include "SensorManager.h"
#include "DisplayManager.h"
#include "RelayController.h"
#include "ConfigManager_RP2040.h"
#include "MenuSystem.h"
#include "InputHandler.h"

// Global objects
PIDController pid;
SensorManager sensors;
DisplayManager display;
RelayController relayController;
ConfigManager configManager;
MenuSystem menu;
InputHandler input;

// Variables
double currentInput = 0.0;
double currentOutput = 0.0;

// Timing
unsigned long lastPIDUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastInputUpdate = 0;
unsigned long lastStatusLED = 0;

const unsigned int PID_UPDATE_INTERVAL = 100;      // 100ms
const unsigned int DISPLAY_UPDATE_INTERVAL = 200;  // 200ms
const unsigned int INPUT_UPDATE_INTERVAL = 10;     // 10ms (fast for encoder)

// Mode flags
bool menuActive = false;

// Function declarations
void updatePID();
void updateDisplay();
void updateInput();
void updateStatusLED();
void enterMenu();
void exitMenu();

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n\n========================================");
    Serial.println(DEVICE_NAME);
    Serial.println("Firmware v" FIRMWARE_VERSION);
    Serial.println("========================================\n");

    // Initialize Status LED
    pinMode(LED_STATUS, OUTPUT);
    digitalWrite(LED_STATUS, HIGH);

    // Initialize Configuration Manager
    Serial.println("Initializing configuration...");
    configManager.begin();

    // Load configuration
    PIDConfig pidConfig = configManager.getPIDConfig();
    SensorConfig sensorConfig = configManager.getSensorConfig();
    RelayConfig relayConfig = configManager.getRelayConfig();

    // Initialize Display
    Serial.println("Initializing display...");
    display.begin();
    display.drawMessage("Initializing...");

    // Initialize Input Handler
    Serial.println("Initializing input...");
    input.begin();

    // Initialize Sensors
    Serial.println("Initializing sensors...");
    sensors.begin();

    // Apply sensor configuration
    if (sensorConfig.activeSensor < 6) {
        sensors.setActiveSensor((SensorType)sensorConfig.activeSensor);
    }
    sensors.setCalibration(sensorConfig.calOffset, sensorConfig.calScale);
    sensors.enableCalibration(sensorConfig.calEnabled);

    // Initialize PID Controller
    Serial.println("Initializing PID controller...");
    pid.begin(pidConfig.kp, pidConfig.ki, pidConfig.kd, pidConfig.setpoint);
    pid.setOutputLimits(0, 255);
    pid.setMode(pidConfig.autoMode);

    // Setup PWM output
    analogWriteFreq(5000);  // 5kHz PWM
    analogWriteResolution(8); // 8-bit (0-255)
    pinMode(PID_OUTPUT_PIN, OUTPUT);

    // Initialize Relay Controller
    Serial.println("Initializing relay controller...");
    relayController.begin();
    relayController.setMode((RelayMode)relayConfig.mode);
    relayController.setHysteresis(relayConfig.hysteresis);
    relayController.setMinCycleTime(relayConfig.minCycleTime);

    if (relayConfig.timeProportioning) {
        relayController.enableTimeProportioning(true, relayConfig.windowSize);
    }

    // Initialize Menu System
    Serial.println("Initializing menu system...");
    menu.begin(&display, &pid, &sensors, &relayController);

    // Show main screen
    display.setPage(PAGE_MAIN);
    menuActive = false;

    Serial.println("\n========================================");
    Serial.println("System Ready!");
    Serial.println("========================================");
    Serial.printf("Active Sensor: %s\n", sensors.getSensorName().c_str());
    Serial.printf("PID Mode: %s\n", pid.isAuto() ? "AUTO" : "MANUAL");
    Serial.println("========================================\n");

    lastPIDUpdate = millis();
    lastDisplayUpdate = millis();
    lastInputUpdate = millis();
    lastStatusLED = millis();
}

void loop() {
    unsigned long now = millis();

    // Update Input (high frequency for responsive encoder)
    if (now - lastInputUpdate >= INPUT_UPDATE_INTERVAL) {
        updateInput();
        lastInputUpdate = now;
    }

    // Update sensors periodically
    sensors.update();

    // Get current input from active sensor
    if (sensors.isDHTAvailable() || sensors.isBMEAvailable() ||
        sensors.isMAX31855Available() || sensors.isMAX6675Available() ||
        sensors.isDS18B20Available()) {
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

    // Update Status LED
    if (now - lastStatusLED >= 500) {
        updateStatusLED();
        lastStatusLED = now;
    }

    // Update menu system if active
    if (menuActive) {
        menu.update();
    }

    delay(1); // Small delay for stability
}

void updatePID() {
    if (pid.isAuto()) {
        currentOutput = pid.compute(currentInput);

        // Apply output to PWM
        analogWrite(PID_OUTPUT_PIN, (int)currentOutput);

        // Update relay controller
        relayController.update(currentInput, pid.getSetpoint(), currentOutput);
    }
}

void updateDisplay() {
    if (menuActive) {
        // Menu system handles display updates
        return;
    }

    // Show main screen
    switch (display.getCurrentPage()) {
        case PAGE_MAIN:
            display.showMainScreen(currentInput, currentOutput,
                                  pid.getSetpoint(), pid.isAuto());
            break;

        case PAGE_SENSORS:
            display.showSensorsScreen(
                sensors.getTemperature(),
                sensors.getHumidity(),
                sensors.getEncoderPosition()
            );
            break;

        default:
            display.setPage(PAGE_MAIN);
            break;
    }
}

void updateInput() {
    input.update();

    int encoderDelta = input.getEncoderDelta();
    bool btnEnter = input.isEnterPressed();
    bool btnBack = input.isBackPressed();

    // Long press Enter to toggle menu
    if (input.isEnterHeld()) {
        static unsigned long enterHeldStart = 0;
        if (enterHeldStart == 0) {
            enterHeldStart = millis();
        }

        if (millis() - enterHeldStart > 1000) { // 1 second hold
            if (menuActive) {
                exitMenu();
            } else {
                enterMenu();
            }
            enterHeldStart = 0;
            while (input.isEnterHeld()) {
                input.update();
                delay(10);
            }
        }
    } else {
        // Reset hold timer
        static unsigned long enterHeldStart = 0;
        enterHeldStart = 0;
    }

    if (menuActive) {
        // Pass input to menu system
        menu.handleInput(encoderDelta, btnEnter, btnBack);
    } else {
        // In main screen, encoder adjusts setpoint
        if (encoderDelta != 0) {
            double newSetpoint = pid.getSetpoint() + (encoderDelta * 0.5);
            if (newSetpoint < 0) newSetpoint = 0;
            if (newSetpoint > 200) newSetpoint = 200;
            pid.setSetpoint(newSetpoint);
        }

        // Enter button toggles PID mode
        if (btnEnter) {
            pid.setMode(!pid.isAuto());
            Serial.printf("PID Mode: %s\n", pid.isAuto() ? "AUTO" : "MANUAL");
        }

        // Back button enters menu (alternative to long press)
        if (btnBack) {
            enterMenu();
        }
    }
}

void updateStatusLED() {
    static bool ledState = false;

    if (pid.isAuto()) {
        // Blink when in AUTO mode
        ledState = !ledState;
        digitalWrite(LED_STATUS, ledState);
    } else {
        // Solid ON in MANUAL mode
        digitalWrite(LED_STATUS, HIGH);
    }
}

void enterMenu() {
    menuActive = true;
    menu.resetTimeout();
    Serial.println("[Main] Entered menu");
}

void exitMenu() {
    menuActive = false;
    display.setPage(PAGE_MAIN);
    Serial.println("[Main] Exited menu");
}
