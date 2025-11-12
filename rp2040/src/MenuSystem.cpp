#include "MenuSystem.h"
#include "DisplayManager.h"
#include "PIDController.h"
#include "SensorManager.h"
#include "RelayController.h"
#include "ConfigManager.h"

extern ConfigManager configManager;

MenuSystem* MenuSystem::instance = nullptr;

MenuSystem::MenuSystem()
    : display(nullptr), pid(nullptr), sensors(nullptr), relays(nullptr),
      currentState(MENU_MAIN), previousState(MENU_MAIN),
      selectedIndex(0), scrollOffset(0),
      editMode(EDIT_NONE), editValue(0), editStep(0.1),
      editMin(0), editMax(100),
      lastActivity(0), lastBlink(0), blinkState(false) {
    instance = this;
}

void MenuSystem::begin(DisplayManager* disp, PIDController* p, SensorManager* s, RelayController* r) {
    display = disp;
    pid = p;
    sensors = s;
    relays = r;

    currentState = MENU_MAIN;
    selectedIndex = 0;
    resetTimeout();

    Serial.println("[Menu] System initialized");
}

void MenuSystem::update() {
    // Blink cursor in edit mode
    if (editMode != EDIT_NONE) {
        if (millis() - lastBlink > 500) {
            blinkState = !blinkState;
            lastBlink = millis();
            renderCurrentMenu();
        }
    }

    // Timeout to main menu
    if (currentState != MENU_MAIN && isTimeout()) {
        Serial.println("[Menu] Timeout - returning to main");
        currentState = MENU_MAIN;
        selectedIndex = 0;
        renderCurrentMenu();
    }
}

void MenuSystem::handleInput(int encoder, bool btnEnter, bool btnBack) {
    resetTimeout();

    if (btnBack) {
        goBack();
        return;
    }

    if (btnEnter) {
        if (editMode != EDIT_NONE) {
            confirmEdit();
        } else {
            selectItem();
        }
        return;
    }

    if (encoder != 0) {
        if (editMode != EDIT_NONE) {
            updateEdit(encoder);
        } else {
            navigate(encoder);
        }
    }
}

void MenuSystem::navigate(int direction) {
    int maxIndex = 0;

    switch (currentState) {
        case MENU_MAIN:
            maxIndex = 6; // 7 items
            break;
        case MENU_PID_SETTINGS:
            maxIndex = 5;
            break;
        case MENU_SENSOR_SELECT:
            maxIndex = 5;
            break;
        case MENU_RELAY_SETTINGS:
            maxIndex = 4;
            break;
        case MENU_SAVE_LOAD:
            maxIndex = 3;
            break;
        default:
            maxIndex = 3;
            break;
    }

    selectedIndex += direction;
    if (selectedIndex < 0) selectedIndex = 0;
    if (selectedIndex > maxIndex) selectedIndex = maxIndex;

    renderCurrentMenu();
}

void MenuSystem::selectItem() {
    Serial.printf("[Menu] Selected item %d in state %d\n", selectedIndex, currentState);

    switch (currentState) {
        case MENU_MAIN:
            switch (selectedIndex) {
                case 0: // PID Settings
                    previousState = currentState;
                    currentState = MENU_PID_SETTINGS;
                    selectedIndex = 0;
                    break;
                case 1: // Sensor Select
                    previousState = currentState;
                    currentState = MENU_SENSOR_SELECT;
                    selectedIndex = 0;
                    break;
                case 2: // Calibration
                    previousState = currentState;
                    currentState = MENU_SENSOR_CALIBRATE;
                    selectedIndex = 0;
                    break;
                case 3: // Relay Settings
                    previousState = currentState;
                    currentState = MENU_RELAY_SETTINGS;
                    selectedIndex = 0;
                    break;
                case 4: // System Info
                    previousState = currentState;
                    currentState = MENU_SYSTEM_INFO;
                    selectedIndex = 0;
                    break;
                case 5: // Save/Load
                    previousState = currentState;
                    currentState = MENU_SAVE_LOAD;
                    selectedIndex = 0;
                    break;
                case 6: // Back to main screen
                    currentState = MENU_MAIN;
                    // Signal to return to main display
                    break;
            }
            break;

        case MENU_PID_SETTINGS:
            switch (selectedIndex) {
                case 0: // Toggle Auto/Manual
                    actionTogglePID();
                    break;
                case 1: // Edit Setpoint
                    startEdit(EDIT_SETPOINT, pid->getSetpoint(), 0.5, 0, 200);
                    break;
                case 2: // Edit Kp
                    startEdit(EDIT_KP, pid->getKp(), 0.1, 0, 100);
                    break;
                case 3: // Edit Ki
                    startEdit(EDIT_KI, pid->getKi(), 0.1, 0, 100);
                    break;
                case 4: // Edit Kd
                    startEdit(EDIT_KD, pid->getKd(), 0.1, 0, 100);
                    break;
                case 5: // Reset PID
                    actionResetPID();
                    break;
            }
            break;

        case MENU_SENSOR_SELECT:
            // Set active sensor based on selection
            if (selectedIndex < 5) {
                sensors->setActiveSensor((SensorType)selectedIndex);
                goBack();
            }
            break;

        case MENU_SENSOR_CALIBRATE:
            switch (selectedIndex) {
                case 0: // Edit Offset
                    {
                        float offset, scale;
                        sensors->getCalibration(offset, scale);
                        startEdit(EDIT_CAL_OFFSET, offset, 0.1, -50, 50);
                    }
                    break;
                case 1: // Edit Scale
                    {
                        float offset, scale;
                        sensors->getCalibration(offset, scale);
                        startEdit(EDIT_CAL_SCALE, scale, 0.01, 0.5, 1.5);
                    }
                    break;
                case 2: // Toggle calibration
                    sensors->enableCalibration(!sensors->isCalibrationEnabled());
                    break;
                case 3: // Start auto-cal
                    // Will need reference value input
                    startEdit(EDIT_CAL_REFERENCE, 25.0, 0.5, -50, 200);
                    break;
            }
            break;

        case MENU_RELAY_SETTINGS:
            switch (selectedIndex) {
                case 0: // Relay mode (cycle through)
                    {
                        RelayMode mode = relays->getMode();
                        int nextMode = ((int)mode + 1) % 4;
                        relays->setMode((RelayMode)nextMode);
                    }
                    break;
                case 1: // Edit hysteresis
                    startEdit(EDIT_HYSTERESIS, relays->getHysteresis(), 0.1, 0.1, 10.0);
                    break;
                case 2: // Toggle time proportioning
                    // Implementation specific
                    break;
                case 3: // Manual relay control
                    // Sub-menu for manual control
                    break;
            }
            break;

        case MENU_SAVE_LOAD:
            switch (selectedIndex) {
                case 0: actionSaveConfig(); break;
                case 1: actionLoadConfig(); break;
                case 2: actionResetConfig(); break;
                case 3: actionReboot(); break;
            }
            break;
    }

    renderCurrentMenu();
}

void MenuSystem::goBack() {
    if (editMode != EDIT_NONE) {
        cancelEdit();
    } else {
        currentState = previousState;
        if (currentState == MENU_MAIN) {
            selectedIndex = 0;
        }
        renderCurrentMenu();
    }
}

void MenuSystem::resetTimeout() {
    lastActivity = millis();
}

bool MenuSystem::isTimeout() const {
    return (millis() - lastActivity) > MENU_TIMEOUT;
}

void MenuSystem::startEdit(EditMode mode, double currentValue, double step, double min, double max) {
    editMode = mode;
    editValue = currentValue;
    editStep = step;
    editMin = min;
    editMax = max;
    blinkState = true;
    lastBlink = millis();

    Serial.printf("[Menu] Start edit mode %d, value=%.2f\n", mode, currentValue);
    renderCurrentMenu();
}

void MenuSystem::updateEdit(int direction) {
    editValue += direction * editStep;
    if (editValue < editMin) editValue = editMin;
    if (editValue > editMax) editValue = editMax;

    renderCurrentMenu();
}

void MenuSystem::confirmEdit() {
    Serial.printf("[Menu] Confirm edit: %.2f\n", editValue);

    switch (editMode) {
        case EDIT_SETPOINT:
            pid->setSetpoint(editValue);
            configManager.setSetpoint(editValue);
            break;
        case EDIT_KP:
            pid->setTunings(editValue, pid->getKi(), pid->getKd());
            break;
        case EDIT_KI:
            pid->setTunings(pid->getKp(), editValue, pid->getKd());
            break;
        case EDIT_KD:
            pid->setTunings(pid->getKp(), pid->getKi(), editValue);
            break;
        case EDIT_HYSTERESIS:
            relays->setHysteresis(editValue);
            break;
        case EDIT_CAL_OFFSET:
            {
                float offset, scale;
                sensors->getCalibration(offset, scale);
                sensors->setCalibration(editValue, scale);
            }
            break;
        case EDIT_CAL_SCALE:
            {
                float offset, scale;
                sensors->getCalibration(offset, scale);
                sensors->setCalibration(offset, editValue);
            }
            break;
        case EDIT_CAL_REFERENCE:
            sensors->startAutoCalibration(editValue);
            break;
        default:
            break;
    }

    editMode = EDIT_NONE;
    renderCurrentMenu();
}

void MenuSystem::cancelEdit() {
    Serial.println("[Menu] Cancel edit");
    editMode = EDIT_NONE;
    renderCurrentMenu();
}

void MenuSystem::renderCurrentMenu() {
    switch (currentState) {
        case MENU_MAIN: renderMainMenu(); break;
        case MENU_PID_SETTINGS: renderPIDSettings(); break;
        case MENU_SENSOR_SELECT: renderSensorSelect(); break;
        case MENU_SENSOR_CALIBRATE: renderSensorCalibrate(); break;
        case MENU_RELAY_SETTINGS: renderRelaySettings(); break;
        case MENU_SYSTEM_INFO: renderSystemInfo(); break;
        case MENU_SAVE_LOAD: renderSaveLoad(); break;
        case MENU_EDIT_VALUE: renderEditValue(); break;
    }
}

void MenuSystem::renderMainMenu() {
    // Will be implemented in DisplayManager
    display->showMenu("Main Menu", selectedIndex, {
        "PID Settings",
        "Sensor Select",
        "Calibration",
        "Relay Settings",
        "System Info",
        "Save/Load",
        "Exit Menu"
    });
}

void MenuSystem::renderPIDSettings() {
    char values[6][32];
    snprintf(values[0], 32, "%s", pid->isAuto() ? "AUTO" : "MANUAL");
    snprintf(values[1], 32, "%.1f", pid->getSetpoint());
    snprintf(values[2], 32, "%.2f", pid->getKp());
    snprintf(values[3], 32, "%.2f", pid->getKi());
    snprintf(values[4], 32, "%.2f", pid->getKd());
    snprintf(values[5], 32, "Reset");

    display->showMenuWithValues("PID Settings", selectedIndex, {
        "Mode",
        "Setpoint",
        "Kp",
        "Ki",
        "Kd",
        "Reset PID"
    }, {
        values[0], values[1], values[2], values[3], values[4], values[5]
    }, editMode != EDIT_NONE ? selectedIndex : -1, blinkState);
}

void MenuSystem::renderSensorSelect() {
    char current[32];
    snprintf(current, 32, "Current: %s", sensors->getSensorName().c_str());

    display->showMenuWithHeader("Select Sensor", current, selectedIndex, {
        "DHT22",
        "BME280",
        "MAX31855",
        "MAX6675",
        "DS18B20",
        "Back"
    });
}

void MenuSystem::renderSensorCalibrate() {
    char values[4][32];
    float offset, scale;
    sensors->getCalibration(offset, scale);

    snprintf(values[0], 32, "%.2f", offset);
    snprintf(values[1], 32, "%.3f", scale);
    snprintf(values[2], 32, "%s", sensors->isCalibrationEnabled() ? "ON" : "OFF");
    snprintf(values[3], 32, "%s", sensors->isAutoCalibrating() ? "Running..." : "Start");

    display->showMenuWithValues("Calibration", selectedIndex, {
        "Offset",
        "Scale",
        "Enabled",
        "Auto-Cal"
    }, {
        values[0], values[1], values[2], values[3]
    }, editMode != EDIT_NONE ? selectedIndex : -1, blinkState);
}

void MenuSystem::renderRelaySettings() {
    char values[4][32];
    const char* modeNames[] = {"OFF", "HEAT", "COOL", "DUAL"};

    snprintf(values[0], 32, "%s", modeNames[(int)relays->getMode()]);
    snprintf(values[1], 32, "%.1f", relays->getHysteresis());
    snprintf(values[2], 32, "TBD");
    snprintf(values[3], 32, "Manual");

    display->showMenuWithValues("Relay Settings", selectedIndex, {
        "Mode",
        "Hysteresis",
        "Time Prop",
        "Manual Ctrl"
    }, {
        values[0], values[1], values[2], values[3]
    }, editMode != EDIT_NONE ? selectedIndex : -1, blinkState);
}

void MenuSystem::renderSystemInfo() {
    char info[8][32];
    snprintf(info[0], 32, DEVICE_NAME);
    snprintf(info[1], 32, "v%s", FIRMWARE_VERSION);
    snprintf(info[2], 32, "%.1f C", sensors->getTemperature());
    snprintf(info[3], 32, "%lu ms", millis());
    snprintf(info[4], 32, "%d KB", rp2040.getFreeHeap() / 1024);
    snprintf(info[5], 32, "%s", sensors->getSensorName().c_str());

    display->showInfoScreen("System Info", {
        {"Device:", info[0]},
        {"Version:", info[1]},
        {"Temp:", info[2]},
        {"Uptime:", info[3]},
        {"Free RAM:", info[4]},
        {"Sensor:", info[5]}
    });
}

void MenuSystem::renderSaveLoad() {
    display->showMenu("Save/Load", selectedIndex, {
        "Save Config",
        "Load Config",
        "Reset to Default",
        "Reboot System"
    });
}

void MenuSystem::renderEditValue() {
    char valueStr[32];
    snprintf(valueStr, 32, "%.2f", editValue);

    const char* label = "Value";
    switch (editMode) {
        case EDIT_SETPOINT: label = "Setpoint"; break;
        case EDIT_KP: label = "Kp"; break;
        case EDIT_KI: label = "Ki"; break;
        case EDIT_KD: label = "Kd"; break;
        case EDIT_HYSTERESIS: label = "Hysteresis"; break;
        case EDIT_CAL_OFFSET: label = "Offset"; break;
        case EDIT_CAL_SCALE: label = "Scale"; break;
        case EDIT_CAL_REFERENCE: label = "Reference"; break;
        default: break;
    }

    display->showEditScreen(label, valueStr, blinkState);
}

// Static action methods
void MenuSystem::actionTogglePID() {
    if (instance && instance->pid) {
        instance->pid->setMode(!instance->pid->isAuto());
        Serial.printf("[Menu] PID mode: %s\n", instance->pid->isAuto() ? "AUTO" : "MANUAL");
    }
}

void MenuSystem::actionResetPID() {
    if (instance && instance->pid) {
        instance->pid->reset();
        Serial.println("[Menu] PID reset");
    }
}

void MenuSystem::actionStartAutoCal() {
    if (instance && instance->sensors) {
        // Reference value should come from edit
        instance->sensors->startAutoCalibration(25.0);
        Serial.println("[Menu] Auto-calibration started");
    }
}

void MenuSystem::actionSaveConfig() {
    configManager.saveConfig();
    Serial.println("[Menu] Configuration saved");
}

void MenuSystem::actionLoadConfig() {
    configManager.loadConfig();
    Serial.println("[Menu] Configuration loaded");
}

void MenuSystem::actionResetConfig() {
    // Reset to defaults and save
    Serial.println("[Menu] Reset to defaults");
}

void MenuSystem::actionReboot() {
    Serial.println("[Menu] Rebooting...");
    delay(1000);
    rp2040.reboot();
}
