#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>
#include "config.h"

enum RelayMode {
    RELAY_OFF,
    RELAY_HEATING_ONLY,
    RELAY_COOLING_ONLY,
    RELAY_DUAL_MODE
};

class RelayController {
private:
    RelayMode mode;
    bool relay1State;  // Heating relay
    bool relay2State;  // Cooling relay
    bool relay3State;  // Aux relay
    bool relay4State;  // Spare relay

    double hysteresis;
    unsigned long lastSwitchTime;
    unsigned long minCycleTime;

    // Time proportioning (for better control)
    bool timeProportioning;
    unsigned long windowSize;
    unsigned long windowStartTime;

public:
    RelayController();
    void begin();

    // Control methods
    void update(double currentValue, double setpoint, double pidOutput);
    void setMode(RelayMode newMode);
    void setHysteresis(double value);
    void setMinCycleTime(unsigned long ms);

    // Time proportioning
    void enableTimeProportioning(bool enable, unsigned long windowMs = 5000);

    // Manual control
    void setRelay(uint8_t relayNum, bool state);
    bool getRelayState(uint8_t relayNum) const;

    // Getters
    RelayMode getMode() const { return mode; }
    double getHysteresis() const { return hysteresis; }

private:
    void updateHeatingOnly(double error, double pidOutput);
    void updateCoolingOnly(double error, double pidOutput);
    void updateDualMode(double error, double pidOutput);
    void updateTimeProportioning(double pidOutput);

    void activateRelay(uint8_t relayNum, bool state);
};

#endif // RELAY_CONTROLLER_H
