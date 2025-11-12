#include "InputHandler.h"

InputHandler::InputHandler()
    : encoder(nullptr),
      btnEnterState(false), btnBackState(false),
      btnUpState(false), btnDownState(false),
      btnEnterPrev(false), btnBackPrev(false),
      btnUpPrev(false), btnDownPrev(false),
      lastBtnEnterTime(0), lastBtnBackTime(0),
      lastBtnUpTime(0), lastBtnDownTime(0),
      lastEncoderPos(0), encoderDelta(0) {
}

void InputHandler::begin() {
    // Initialize encoder
    encoder = new RotaryEncoder(ENCODER_PIN_A, ENCODER_PIN_B, RotaryEncoder::LatchMode::FOUR3);

    // Initialize button pins
    pinMode(ENCODER_BTN, INPUT_PULLUP);
    pinMode(BTN_BACK, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);

    Serial.println("[Input] Handler initialized");
}

void InputHandler::update() {
    // Update encoder
    encoder->tick();

    int newPos = encoder->getPosition();
    encoderDelta = newPos - lastEncoderPos;
    lastEncoderPos = newPos;

    // Update button states with debounce
    readButton(ENCODER_BTN, btnEnterState, btnEnterPrev, lastBtnEnterTime);
    readButton(BTN_BACK, btnBackState, btnBackPrev, lastBtnBackTime);
    readButton(BTN_UP, btnUpState, btnUpPrev, lastBtnUpTime);
    readButton(BTN_DOWN, btnDownState, btnDownPrev, lastBtnDownTime);
}

int InputHandler::getEncoderDelta() {
    int delta = encoderDelta;
    encoderDelta = 0; // Reset after reading
    return delta / ENCODER_SENSITIVITY;
}

bool InputHandler::isEnterPressed() {
    return btnEnterState && !btnEnterPrev;
}

bool InputHandler::isBackPressed() {
    return btnBackState && !btnBackPrev;
}

bool InputHandler::isUpPressed() {
    return btnUpState && !btnUpPrev;
}

bool InputHandler::isDownPressed() {
    return btnDownState && !btnDownPrev;
}

bool InputHandler::isEnterHeld() {
    return btnEnterState;
}

bool InputHandler::isBackHeld() {
    return btnBackState;
}

bool InputHandler::readButton(uint8_t pin, bool& currentState, bool& prevState, unsigned long& lastTime) {
    bool reading = !digitalRead(pin); // Active LOW (pullup)

    if (reading != currentState) {
        if ((millis() - lastTime) > DEBOUNCE_DELAY) {
            prevState = currentState;
            currentState = reading;
            lastTime = millis();
            return true; // State changed
        }
    }

    return false; // No change
}
