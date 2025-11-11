#include "PIDController.h"

PIDController::PIDController()
    : kp(0), ki(0), kd(0), setpoint(0),
      outputMin(0), outputMax(255),
      integral(0), previousError(0),
      lastTime(0), sampleTime(100),
      autoMode(false) {
}

void PIDController::begin(double Kp, double Ki, double Kd, double Setpoint) {
    setTunings(Kp, Ki, Kd);
    setSetpoint(Setpoint);
    reset();
    autoMode = true;
    lastTime = millis();
}

void PIDController::setTunings(double Kp, double Ki, double Kd) {
    if (Kp < 0 || Ki < 0 || Kd < 0) return;

    kp = Kp;
    ki = Ki;
    kd = Kd;
}

void PIDController::setSetpoint(double Setpoint) {
    setpoint = Setpoint;
}

void PIDController::setOutputLimits(double Min, double Max) {
    if (Min >= Max) return;
    outputMin = Min;
    outputMax = Max;

    // Clamp existing integral if needed
    if (integral > outputMax) integral = outputMax;
    else if (integral < outputMin) integral = outputMin;
}

void PIDController::setSampleTime(unsigned int NewSampleTime) {
    if (NewSampleTime > 0) {
        sampleTime = NewSampleTime;
    }
}

void PIDController::setMode(bool Mode) {
    if (Mode && !autoMode) {
        // Switching from manual to auto
        reset();
    }
    autoMode = Mode;
}

double PIDController::compute(double input) {
    if (!autoMode) return 0;

    unsigned long now = millis();
    unsigned long timeChange = now - lastTime;

    if (timeChange >= sampleTime) {
        // Calculate error
        double error = setpoint - input;

        // Calculate time in seconds
        double dt = timeChange / 1000.0;

        // Proportional term
        double pTerm = kp * error;

        // Integral term (with anti-windup)
        integral += (ki * error * dt);
        if (integral > outputMax) integral = outputMax;
        else if (integral < outputMin) integral = outputMin;

        // Derivative term
        double dTerm = 0;
        if (dt > 0) {
            dTerm = kd * (error - previousError) / dt;
        }

        // Calculate output
        double output = pTerm + integral + dTerm;

        // Clamp output
        if (output > outputMax) output = outputMax;
        else if (output < outputMin) output = outputMin;

        // Remember for next time
        previousError = error;
        lastTime = now;

        return output;
    }

    return 0; // Not time to compute yet
}

void PIDController::reset() {
    integral = 0;
    previousError = 0;
    lastTime = millis();
}
