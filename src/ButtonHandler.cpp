#include "ButtonHandler.h"

ButtonHandler::ButtonHandler()
    : pinUp(BTN_UP), pinDown(BTN_DOWN), pinSelect(BTN_SELECT), pinBack(BTN_BACK),
      upState(false), downState(false), selectState(false), backState(false),
      upLastState(false), downLastState(false), selectLastState(false), backLastState(false),
      upLastDebounce(0), downLastDebounce(0), selectLastDebounce(0), backLastDebounce(0),
      debounceDelay(BUTTON_DEBOUNCE_MS),
      upPressStart(0), downPressStart(0), selectPressStart(0), backPressStart(0),
      longPressTime(BUTTON_LONG_PRESS_MS),
      upLongPressed(false), downLongPressed(false),
      selectLongPressed(false), backLongPressed(false),
      upPressed(false), downPressed(false), selectPressed(false), backPressed(false) {
}

void ButtonHandler::begin(uint8_t up, uint8_t down, uint8_t select, uint8_t back) {
    pinUp = up;
    pinDown = down;
    pinSelect = select;
    pinBack = back;

    // Configure pins as inputs with internal pull-up resistors
    pinMode(pinUp, INPUT_PULLUP);
    pinMode(pinDown, INPUT_PULLUP);
    pinMode(pinSelect, INPUT_PULLUP);
    pinMode(pinBack, INPUT_PULLUP);

    Serial.println("[Buttons] Initialized with pins:");
    Serial.printf("  UP=%d, DOWN=%d, SELECT=%d, BACK=%d\n", pinUp, pinDown, pinSelect, pinBack);
}

void ButtonHandler::update() {
    // Update all buttons with debouncing
    debounceButton(pinUp, upState, upLastState, upLastDebounce, upPressed, upPressStart, upLongPressed);
    debounceButton(pinDown, downState, downLastState, downLastDebounce, downPressed, downPressStart, downLongPressed);
    debounceButton(pinSelect, selectState, selectLastState, selectLastDebounce, selectPressed, selectPressStart, selectLongPressed);
    debounceButton(pinBack, backState, backLastState, backLastDebounce, backPressed, backPressStart, backLongPressed);
}

bool ButtonHandler::debounceButton(uint8_t pin, bool &currentState, bool &lastState,
                                   unsigned long &lastDebounce, bool &pressedFlag,
                                   unsigned long &pressStart, bool &longPressedFlag) {
    // Read the button state (LOW = pressed because of INPUT_PULLUP)
    bool reading = (digitalRead(pin) == LOW);

    // If the button state changed, reset the debounce timer
    if (reading != lastState) {
        lastDebounce = millis();
    }

    // If enough time has passed since last change
    if ((millis() - lastDebounce) > debounceDelay) {
        // If the button state has changed after debounce
        if (reading != currentState) {
            currentState = reading;

            // Button was just pressed (edge detection)
            if (currentState) {
                pressedFlag = true;
                pressStart = millis();
                longPressedFlag = false;  // Reset long press flag
            } else {
                // Button was released
                pressStart = 0;
            }
        }

        // Check for long press (button held down)
        if (currentState && !longPressedFlag && pressStart > 0) {
            if ((millis() - pressStart) >= longPressTime) {
                longPressedFlag = true;
            }
        }
    }

    lastState = reading;
    return currentState;
}

bool ButtonHandler::anyButtonPressed() const {
    return upState || downState || selectState || backState;
}
