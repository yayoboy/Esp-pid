#include "RelayController.h"

RelayController::RelayController()
    : mode(RELAY_OFF),
      relay1State(false), relay2State(false),
      relay3State(false), relay4State(false),
      hysteresis(RELAY_HYSTERESIS),
      lastSwitchTime(0),
      minCycleTime(RELAY_MIN_CYCLE),
      timeProportioning(false),
      windowSize(5000),
      windowStartTime(0) {
}

void RelayController::begin() {
    // Initialize relay pins as outputs
    pinMode(RELAY_PIN_1, OUTPUT);
    pinMode(RELAY_PIN_2, OUTPUT);
    pinMode(RELAY_PIN_3, OUTPUT);
    pinMode(RELAY_PIN_4, OUTPUT);

    // Set all relays to OFF initially
    digitalWrite(RELAY_PIN_1, LOW);
    digitalWrite(RELAY_PIN_2, LOW);
    digitalWrite(RELAY_PIN_3, LOW);
    digitalWrite(RELAY_PIN_4, LOW);

    Serial.println("[✓] Relay controller initialized");
    Serial.printf("    Mode: %d, Hysteresis: %.2f\n", mode, hysteresis);
}

void RelayController::update(double currentValue, double setpoint, double pidOutput) {
    if (mode == RELAY_OFF) return;

    double error = setpoint - currentValue;

    // Check minimum cycle time
    unsigned long now = millis();
    if (now - lastSwitchTime < minCycleTime) {
        return; // Too soon to switch
    }

    if (timeProportioning) {
        updateTimeProportioning(pidOutput);
    } else {
        switch (mode) {
            case RELAY_HEATING_ONLY:
                updateHeatingOnly(error, pidOutput);
                break;
            case RELAY_COOLING_ONLY:
                updateCoolingOnly(error, pidOutput);
                break;
            case RELAY_DUAL_MODE:
                updateDualMode(error, pidOutput);
                break;
            default:
                break;
        }
    }
}

void RelayController::updateHeatingOnly(double error, double pidOutput) {
    // Simple hysteresis control
    if (error > hysteresis && !relay1State) {
        activateRelay(1, true);
    } else if (error < -hysteresis && relay1State) {
        activateRelay(1, false);
    }
}

void RelayController::updateCoolingOnly(double error, double pidOutput) {
    // Cooling is opposite: turn on when too hot
    if (error < -hysteresis && !relay2State) {
        activateRelay(2, true);
    } else if (error > hysteresis && relay2State) {
        activateRelay(2, false);
    }
}

void RelayController::updateDualMode(double error, double pidOutput) {
    // Dual mode: heating and cooling with deadband

    if (error > hysteresis) {
        // Need heating
        if (!relay1State) activateRelay(1, true);
        if (relay2State) activateRelay(2, false);
    } else if (error < -hysteresis) {
        // Need cooling
        if (relay1State) activateRelay(1, false);
        if (!relay2State) activateRelay(2, true);
    } else {
        // Within deadband - turn both off
        if (relay1State) activateRelay(1, false);
        if (relay2State) activateRelay(2, false);
    }
}

void RelayController::updateTimeProportioning(double pidOutput) {
    // Time proportioning relay control (like slow PWM)
    unsigned long now = millis();

    // Start new window
    if (now - windowStartTime >= windowSize) {
        windowStartTime = now;
    }

    // Calculate on-time based on PID output (0-255)
    unsigned long onTime = (pidOutput / 255.0) * windowSize;

    // Determine relay state based on time in window
    unsigned long elapsed = now - windowStartTime;

    if (mode == RELAY_HEATING_ONLY || mode == RELAY_DUAL_MODE) {
        bool shouldBeOn = (elapsed < onTime);
        if (relay1State != shouldBeOn) {
            activateRelay(1, shouldBeOn);
        }
    }

    if (mode == RELAY_COOLING_ONLY || mode == RELAY_DUAL_MODE) {
        // Cooling is inverse
        bool shouldBeOn = (elapsed > onTime);
        if (relay2State != shouldBeOn) {
            activateRelay(2, shouldBeOn);
        }
    }
}

void RelayController::setMode(RelayMode newMode) {
    mode = newMode;

    // Turn off all relays when changing mode
    activateRelay(1, false);
    activateRelay(2, false);

    Serial.printf("Relay mode set to: %d\n", mode);
}

void RelayController::setHysteresis(double value) {
    hysteresis = value;
    Serial.printf("Relay hysteresis set to: %.2f\n", hysteresis);
}

void RelayController::setMinCycleTime(unsigned long ms) {
    minCycleTime = ms;
    Serial.printf("Relay min cycle time set to: %lu ms\n", minCycleTime);
}

void RelayController::enableTimeProportioning(bool enable, unsigned long windowMs) {
    timeProportioning = enable;
    if (enable) {
        windowSize = windowMs;
        windowStartTime = millis();
        Serial.printf("Time proportioning enabled with window: %lu ms\n", windowMs);
    } else {
        Serial.println("Time proportioning disabled");
    }
}

void RelayController::setRelay(uint8_t relayNum, bool state) {
    activateRelay(relayNum, state);
}

bool RelayController::getRelayState(uint8_t relayNum) const {
    switch (relayNum) {
        case 1: return relay1State;
        case 2: return relay2State;
        case 3: return relay3State;
        case 4: return relay4State;
        default: return false;
    }
}

void RelayController::activateRelay(uint8_t relayNum, bool state) {
    uint8_t pin;
    bool* stateVar;

    switch (relayNum) {
        case 1:
            pin = RELAY_PIN_1;
            stateVar = &relay1State;
            break;
        case 2:
            pin = RELAY_PIN_2;
            stateVar = &relay2State;
            break;
        case 3:
            pin = RELAY_PIN_3;
            stateVar = &relay3State;
            break;
        case 4:
            pin = RELAY_PIN_4;
            stateVar = &relay4State;
            break;
        default:
            return;
    }

    // Update state
    *stateVar = state;
    digitalWrite(pin, state ? HIGH : LOW);
    lastSwitchTime = millis();

    Serial.printf("Relay %d: %s\n", relayNum, state ? "ON" : "OFF");
}
