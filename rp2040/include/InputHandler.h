#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <Arduino.h>
#include <RotaryEncoder.h>
#include "config.h"

class InputHandler {
private:
    RotaryEncoder* encoder;

    // Button states
    bool btnEnterState;
    bool btnBackState;
    bool btnUpState;
    bool btnDownState;

    // Previous button states for edge detection
    bool btnEnterPrev;
    bool btnBackPrev;
    bool btnUpPrev;
    bool btnDownPrev;

    // Debounce timing
    unsigned long lastBtnEnterTime;
    unsigned long lastBtnBackTime;
    unsigned long lastBtnUpTime;
    unsigned long lastBtnDownTime;

    // Encoder position
    int lastEncoderPos;
    int encoderDelta;

public:
    InputHandler();
    void begin();
    void update();

    // Get input states
    int getEncoderDelta();  // Returns change since last call
    bool isEnterPressed();   // Button pressed (edge)
    bool isBackPressed();    // Button pressed (edge)
    bool isUpPressed();      // Button pressed (edge)
    bool isDownPressed();    // Button pressed (edge)

    bool isEnterHeld();      // Button held down
    bool isBackHeld();       // Button held down

private:
    bool readButton(uint8_t pin, bool& currentState, bool& prevState, unsigned long& lastTime);
};

#endif // INPUT_HANDLER_H
