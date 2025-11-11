#include "WebServer.h"
#include "config.h"

WebServerManager::WebServerManager()
    : server(WEB_SERVER_PORT), ws("/ws"),
      pidController(nullptr), sensorManager(nullptr),
      currentInput(nullptr), currentOutput(nullptr) {
}

void WebServerManager::begin(PIDController* pid, SensorManager* sensors, double* input, double* output) {
    pidController = pid;
    sensorManager = sensors;
    currentInput = input;
    currentOutput = output;

    setupRoutes();
    setupWebSocket();

    server.begin();
    Serial.println("Web server started on port 80");
}

void WebServerManager::setupRoutes() {
    // Serve main page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 PID Controller</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            color: #333;
        }
        .container {
            max-width: 900px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.2);
        }
        h1 {
            text-align: center;
            color: #667eea;
            margin-bottom: 30px;
            font-size: 2.5em;
        }
        .card {
            background: #f8f9fa;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .card h2 {
            color: #667eea;
            margin-bottom: 15px;
            font-size: 1.5em;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
        }
        .data-row {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 0;
            border-bottom: 1px solid #e0e0e0;
        }
        .data-row:last-child { border-bottom: none; }
        .label {
            font-weight: bold;
            color: #555;
        }
        .value {
            font-size: 1.3em;
            color: #667eea;
            font-weight: bold;
        }
        .input-group {
            display: flex;
            gap: 10px;
            margin: 10px 0;
        }
        input[type="number"] {
            flex: 1;
            padding: 12px;
            border: 2px solid #ddd;
            border-radius: 5px;
            font-size: 1em;
        }
        input[type="number"]:focus {
            outline: none;
            border-color: #667eea;
        }
        button {
            padding: 12px 25px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1em;
            font-weight: bold;
            transition: transform 0.2s;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }
        button:active { transform: translateY(0); }
        .progress-bar {
            width: 100%;
            height: 30px;
            background: #e0e0e0;
            border-radius: 15px;
            overflow: hidden;
            margin-top: 10px;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #667eea 0%, #764ba2 100%);
            transition: width 0.3s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
        }
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }
        .status-auto { background: #4caf50; }
        .status-manual { background: #f44336; }
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        input:checked + .slider {
            background-color: #667eea;
        }
        input:checked + .slider:before {
            transform: translateX(26px);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎛️ ESP32 PID Controller</h1>

        <div class="card">
            <h2>📊 Real-time Data</h2>
            <div class="data-row">
                <span class="label">Setpoint:</span>
                <span class="value" id="setpoint">0.0</span>
            </div>
            <div class="data-row">
                <span class="label">Current Value:</span>
                <span class="value" id="input">0.0</span>
            </div>
            <div class="data-row">
                <span class="label">Output:</span>
                <span class="value" id="output">0.0%</span>
            </div>
            <div class="progress-bar">
                <div class="progress-fill" id="outputBar" style="width: 0%">0%</div>
            </div>
            <div class="data-row">
                <span class="label">Mode:</span>
                <span class="value">
                    <span class="status-indicator" id="statusIndicator"></span>
                    <span id="mode">Manual</span>
                </span>
            </div>
        </div>

        <div class="card">
            <h2>🌡️ Sensors</h2>
            <div class="data-row">
                <span class="label">Temperature:</span>
                <span class="value" id="temperature">0.0 °C</span>
            </div>
            <div class="data-row">
                <span class="label">Humidity:</span>
                <span class="value" id="humidity">0.0 %</span>
            </div>
            <div class="data-row">
                <span class="label">Position:</span>
                <span class="value" id="position">0</span>
            </div>
        </div>

        <div class="card">
            <h2>⚙️ PID Configuration</h2>
            <div class="input-group">
                <input type="number" id="kp" placeholder="Kp" step="0.1">
                <input type="number" id="ki" placeholder="Ki" step="0.1">
                <input type="number" id="kd" placeholder="Kd" step="0.1">
            </div>
            <button onclick="updatePID()">Update PID</button>
        </div>

        <div class="card">
            <h2>🎯 Control</h2>
            <div class="input-group">
                <input type="number" id="newSetpoint" placeholder="New Setpoint" step="0.1">
                <button onclick="updateSetpoint()">Set</button>
            </div>
            <div class="data-row">
                <span class="label">Auto Mode:</span>
                <label class="toggle-switch">
                    <input type="checkbox" id="autoMode" onchange="toggleMode()">
                    <span class="slider"></span>
                </label>
            </div>
        </div>
    </div>

    <script>
        var ws = new WebSocket('ws://' + window.location.hostname + '/ws');

        ws.onmessage = function(event) {
            var data = JSON.parse(event.data);
            if (data.type === 'status') {
                document.getElementById('setpoint').textContent = data.setpoint.toFixed(1);
                document.getElementById('input').textContent = data.input.toFixed(1);
                var outputPercent = (data.output / 255 * 100).toFixed(1);
                document.getElementById('output').textContent = outputPercent + '%';
                document.getElementById('outputBar').style.width = outputPercent + '%';
                document.getElementById('outputBar').textContent = outputPercent + '%';

                document.getElementById('mode').textContent = data.mode ? 'Auto' : 'Manual';
                var indicator = document.getElementById('statusIndicator');
                indicator.className = 'status-indicator ' + (data.mode ? 'status-auto' : 'status-manual');

                document.getElementById('temperature').textContent = data.temp.toFixed(1) + ' °C';
                document.getElementById('humidity').textContent = data.humidity.toFixed(1) + ' %';
                document.getElementById('position').textContent = data.position;

                document.getElementById('kp').value = data.kp.toFixed(2);
                document.getElementById('ki').value = data.ki.toFixed(2);
                document.getElementById('kd').value = data.kd.toFixed(2);
                document.getElementById('autoMode').checked = data.mode;
            }
        };

        function updatePID() {
            var cmd = {
                cmd: 'setPID',
                kp: parseFloat(document.getElementById('kp').value),
                ki: parseFloat(document.getElementById('ki').value),
                kd: parseFloat(document.getElementById('kd').value)
            };
            ws.send(JSON.stringify(cmd));
        }

        function updateSetpoint() {
            var cmd = {
                cmd: 'setSetpoint',
                value: parseFloat(document.getElementById('newSetpoint').value)
            };
            ws.send(JSON.stringify(cmd));
        }

        function toggleMode() {
            var cmd = {
                cmd: 'setMode',
                value: document.getElementById('autoMode').checked
            };
            ws.send(JSON.stringify(cmd));
        }
    </script>
</body>
</html>
)rawliteral";
        request->send(200, "text/html", html);
    });

    // API endpoint for status
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "application/json", getStatusJSON());
    });

    // API endpoint for config
    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "application/json", getConfigJSON());
    });
}

void WebServerManager::setupWebSocket() {
    ws.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client,
                      AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("WebSocket client #%u connected\n", client->id());
            sendUpdate();
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
        } else if (type == WS_EVT_DATA) {
            handleWebSocketMessage(arg, data, len);
        }
    });

    server.addHandler(&ws);
}

void WebServerManager::handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, (char*)data);

        if (error) {
            Serial.println("Failed to parse WebSocket message");
            return;
        }

        String cmd = doc["cmd"];

        if (cmd == "setPID") {
            double kp = doc["kp"];
            double ki = doc["ki"];
            double kd = doc["kd"];
            pidController->setTunings(kp, ki, kd);
            Serial.printf("PID updated: Kp=%.2f, Ki=%.2f, Kd=%.2f\n", kp, ki, kd);
        } else if (cmd == "setSetpoint") {
            double value = doc["value"];
            pidController->setSetpoint(value);
            Serial.printf("Setpoint updated: %.2f\n", value);
        } else if (cmd == "setMode") {
            bool mode = doc["value"];
            pidController->setMode(mode);
            Serial.printf("Mode changed: %s\n", mode ? "AUTO" : "MANUAL");
        }

        sendUpdate();
    }
}

void WebServerManager::sendUpdate() {
    String json = getStatusJSON();
    ws.textAll(json);
}

String WebServerManager::getStatusJSON() {
    DynamicJsonDocument doc(512);

    doc["type"] = "status";
    doc["setpoint"] = pidController->getSetpoint();
    doc["input"] = currentInput ? *currentInput : 0.0;
    doc["output"] = currentOutput ? *currentOutput : 0.0;
    doc["mode"] = pidController->isAuto();
    doc["kp"] = pidController->getKp();
    doc["ki"] = pidController->getKi();
    doc["kd"] = pidController->getKd();
    doc["temp"] = sensorManager->getTemperature();
    doc["humidity"] = sensorManager->getHumidity();
    doc["position"] = sensorManager->getEncoderPosition();

    String output;
    serializeJson(doc, output);
    return output;
}

String WebServerManager::getConfigJSON() {
    DynamicJsonDocument doc(256);

    doc["kp"] = pidController->getKp();
    doc["ki"] = pidController->getKi();
    doc["kd"] = pidController->getKd();
    doc["setpoint"] = pidController->getSetpoint();

    String output;
    serializeJson(doc, output);
    return output;
}
