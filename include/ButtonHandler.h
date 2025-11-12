#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>
#include "config.h"

// Button states
enum ButtonState {
    BTN_RELEASED,
    BTN_PRESSED,
    BTN_HELD
};

class ButtonHandler {
private:
    // Pin definitions
    uint8_t pinUp;
    uint8_t pinDown;
    uint8_t pinSelect;
    uint8_t pinBack;

    // Button states
    bool upState, downState, selectState, backState;
    bool upLastState, downLastState, selectLastState, backLastState;

    // Debouncing
    unsigned long upLastDebounce, downLastDebounce;
    unsigned long selectLastDebounce, backLastDebounce;
    unsigned long debounceDelay;

    // Long press detection
    unsigned long upPressStart, downPressStart;
    unsigned long selectPressStart, backPressStart;
    unsigned long longPressTime;
    bool upLongPressed, downLongPressed;
    bool selectLongPressed, backLongPressed;

    // Edge detection (for single press)
    bool upPressed, downPressed, selectPressed, backPressed;

public:
    ButtonHandler();

    // Initialize buttons with custom pins (optional)
    void begin(uint8_t up = BTN_UP, uint8_t down = BTN_DOWN,
               uint8_t select = BTN_SELECT, uint8_t back = BTN_BACK);

    // Update button states (call in loop)
    void update();

    // Check if button was just pressed (edge detection)
    bool isUpPressed() { bool result = upPressed; upPressed = false; return result; }
    bool isDownPressed() { bool result = downPressed; downPressed = false; return result; }
    bool isSelectPressed() { bool result = selectPressed; selectPressed = false; return result; }
    bool isBackPressed() { bool result = backPressed; backPressed = false; return result; }

    // Check if button is currently held down
    bool isUpHeld() const { return upState; }
    bool isDownHeld() const { return downState; }
    bool isSelectHeld() const { return selectState; }
    bool isBackHeld() const { return backState; }

    // Check if button was long pressed (held for BUTTON_LONG_PRESS_MS)
    bool isUpLongPressed() { bool result = upLongPressed; upLongPressed = false; return result; }
    bool isDownLongPressed() { bool result = downLongPressed; downLongPressed = false; return result; }
    bool isSelectLongPressed() { bool result = selectLongPressed; selectLongPressed = false; return result; }
    bool isBackLongPressed() { bool result = backLongPressed; backLongPressed = false; return result; }

    // Check any button activity
    bool anyButtonPressed() const;

private:
    // Debounce a single button
    bool debounceButton(uint8_t pin, bool &currentState, bool &lastState,
                       unsigned long &lastDebounce, bool &pressedFlag,
                       unsigned long &pressStart, bool &longPressedFlag);
};

#endif // BUTTON_HANDLER_H
