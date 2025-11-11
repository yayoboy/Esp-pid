#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

#include <Arduino.h>

class PIDController {
private:
    double kp, ki, kd;
    double setpoint;
    double outputMin, outputMax;
    double integral;
    double previousError;
    unsigned long lastTime;
    unsigned int sampleTime;
    bool autoMode;

public:
    PIDController();

    void begin(double Kp, double Ki, double Kd, double Setpoint);
    void setTunings(double Kp, double Ki, double Kd);
    void setSetpoint(double Setpoint);
    void setOutputLimits(double Min, double Max);
    void setSampleTime(unsigned int NewSampleTime);
    void setMode(bool Mode);

    double compute(double input);
    void reset();

    // Getters
    double getKp() const { return kp; }
    double getKi() const { return ki; }
    double getKd() const { return kd; }
    double getSetpoint() const { return setpoint; }
    double getIntegral() const { return integral; }
    bool isAuto() const { return autoMode; }
};

#endif // PID_CONTROLLER_H
