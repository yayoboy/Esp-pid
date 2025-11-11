#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "PIDController.h"
#include "SensorManager.h"

class WebServerManager {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;

    PIDController* pidController;
    SensorManager* sensorManager;

    double* currentInput;
    double* currentOutput;

public:
    WebServerManager();
    void begin(PIDController* pid, SensorManager* sensors, double* input, double* output);

    void sendUpdate();
    void handleWebSocketMessage(void* arg, uint8_t* data, size_t len);

private:
    void setupRoutes();
    void setupWebSocket();

    String getStatusJSON();
    String getConfigJSON();
};

#endif // WEB_SERVER_H
