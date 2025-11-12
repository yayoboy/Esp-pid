#include "WiFiManager.h"
#include "config.h"

WiFiManager::WiFiManager(ConfigManager* configMgr)
    : config(configMgr),
      dnsServer(nullptr),
      captiveServer(nullptr),
      status(WIFI_IDLE),
      connectStartTime(0),
      apStartTime(0),
      resetButtonPressTime(0),
      resetButtonPressed(false) {
}

WiFiManager::~WiFiManager() {
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
    }
    if (captiveServer) {
        captiveServer->end();
        delete captiveServer;
    }
}

bool WiFiManager::begin() {
    // Setup reset button
    pinMode(WIFI_RESET_PIN, INPUT_PULLUP);

    // Load saved credentials from config
    WiFiConfig wifiCfg = config->getWiFiConfig();
    savedSSID = String(wifiCfg.ssid);
    savedPassword = String(wifiCfg.password);

    // Check if we have valid credentials (not default AP credentials)
    if (savedSSID.length() > 0 && savedSSID != AP_SSID) {
        Serial.println("[WiFi] Attempting connection with saved credentials...");
        return connectToWiFi();
    } else {
        Serial.println("[WiFi] No saved credentials - starting captive portal");
        startCaptivePortal();
        return false;
    }
}

void WiFiManager::update() {
    // Check reset button
    checkResetButton();

    // Process DNS requests if in AP mode
    if (status == WIFI_AP_MODE && dnsServer) {
        dnsServer->processNextRequest();
    }

    // Check connection status
    switch (status) {
        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                status = WIFI_CONNECTED;
                Serial.println("[WiFi] Connected!");
                Serial.print("[WiFi] IP Address: ");
                Serial.println(WiFi.localIP());
            } else if (millis() - connectStartTime > WIFI_CONNECT_TIMEOUT) {
                Serial.println("[WiFi] Connection timeout - starting captive portal");
                WiFi.disconnect();
                startCaptivePortal();
            }
            break;

        case WIFI_AP_MODE:
            // Check if timeout reached and should retry connection
            if (savedSSID.length() > 0 &&
                millis() - apStartTime > CAPTIVE_PORTAL_TIMEOUT) {
                Serial.println("[WiFi] Captive portal timeout - retrying saved credentials");
                stopAPMode();
                connectToWiFi();
            }
            break;

        case WIFI_CONNECTED:
            // Check if connection was lost
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("[WiFi] Connection lost - attempting reconnection");
                status = WIFI_CONNECTING;
                connectStartTime = millis();
            }
            break;

        default:
            break;
    }
}

String WiFiManager::getStatusString() const {
    switch (status) {
        case WIFI_IDLE: return "Idle";
        case WIFI_CONNECTING: return "Connecting";
        case WIFI_CONNECTED: return "Connected";
        case WIFI_AP_MODE: return "AP Mode (Setup)";
        case WIFI_FAILED: return "Failed";
        default: return "Unknown";
    }
}

String WiFiManager::getIPAddress() const {
    if (status == WIFI_CONNECTED) {
        return WiFi.localIP().toString();
    } else if (status == WIFI_AP_MODE) {
        return WiFi.softAPIP().toString();
    }
    return "0.0.0.0";
}

void WiFiManager::startCaptivePortal() {
    startAPMode();
}

void WiFiManager::resetCredentials() {
    Serial.println("[WiFi] Resetting credentials...");
    savedSSID = "";
    savedPassword = "";

    // Reset to default AP credentials in config
    WiFiConfig wifiCfg;
    strncpy(wifiCfg.ssid, AP_SSID, sizeof(wifiCfg.ssid));
    strncpy(wifiCfg.password, AP_PASSWORD, sizeof(wifiCfg.password));
    strncpy(wifiCfg.hostname, HOSTNAME, sizeof(wifiCfg.hostname));
    config->setWiFiConfig(wifiCfg);

    // Start captive portal
    WiFi.disconnect();
    startCaptivePortal();
}

void WiFiManager::checkResetButton() {
    // Check if BOOT button (GPIO0) is held for 5 seconds
    if (digitalRead(WIFI_RESET_PIN) == LOW) {
        if (!resetButtonPressed) {
            resetButtonPressed = true;
            resetButtonPressTime = millis();
        } else if (millis() - resetButtonPressTime > 5000) {
            Serial.println("[WiFi] Reset button held - resetting credentials");
            resetCredentials();
            resetButtonPressed = false;
        }
    } else {
        resetButtonPressed = false;
    }
}

void WiFiManager::setupRoutes(AsyncWebServer* server) {
    // Captive portal detection endpoints
    server->on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    server->on("/fwlink", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    // Main captive portal page
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (status == WIFI_AP_MODE) {
            request->send(200, "text/html", getCaptivePortalHTML());
        } else {
            request->send(200, "text/plain", "WiFi already configured. IP: " + getIPAddress());
        }
    });

    // WiFi scan endpoint
    server->on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json", getNetworksJSON());
    });

    // Save WiFi credentials endpoint
    server->on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String ssid = "";
        String password = "";

        if (request->hasParam("ssid", true)) {
            ssid = request->getParam("ssid", true)->value();
        }
        if (request->hasParam("password", true)) {
            password = request->getParam("password", true)->value();
        }

        if (ssid.length() > 0) {
            if (saveCredentials(ssid, password)) {
                request->send(200, "text/html",
                    "<html><body><h1>WiFi Configuration Saved!</h1>"
                    "<p>ESP32 will now restart and connect to: " + ssid + "</p>"
                    "<p>Restarting in 3 seconds...</p>"
                    "<script>setTimeout(function(){window.location.href='/';}, 3000);</script>"
                    "</body></html>");

                // Restart after 3 seconds
                delay(3000);
                ESP.restart();
            } else {
                request->send(500, "text/html",
                    "<html><body><h1>Error!</h1><p>Failed to save credentials</p></body></html>");
            }
        } else {
            request->send(400, "text/html",
                "<html><body><h1>Error!</h1><p>SSID is required</p></body></html>");
        }
    });

    // Reset WiFi endpoint
    server->on("/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        resetCredentials();
        request->send(200, "text/html",
            "<html><body><h1>WiFi Reset!</h1>"
            "<p>Credentials cleared. Restarting captive portal...</p></body></html>");
    });

    // Catch all for captive portal
    server->onNotFound([this](AsyncWebServerRequest *request) {
        if (status == WIFI_AP_MODE) {
            request->redirect("/");
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });
}

bool WiFiManager::saveCredentials(const String& ssid, const String& password) {
    savedSSID = ssid;
    savedPassword = password;

    // Save to config using ConfigManager interface
    WiFiConfig wifiCfg;
    strncpy(wifiCfg.ssid, ssid.c_str(), sizeof(wifiCfg.ssid) - 1);
    wifiCfg.ssid[sizeof(wifiCfg.ssid) - 1] = '\0';
    strncpy(wifiCfg.password, password.c_str(), sizeof(wifiCfg.password) - 1);
    wifiCfg.password[sizeof(wifiCfg.password) - 1] = '\0';
    strncpy(wifiCfg.hostname, HOSTNAME, sizeof(wifiCfg.hostname));

    config->setWiFiConfig(wifiCfg);

    Serial.printf("[WiFi] Saved credentials - SSID: %s\n", ssid.c_str());
    return true;
}

bool WiFiManager::connectToWiFi() {
    if (savedSSID.length() == 0) {
        return false;
    }

    Serial.print("[WiFi] Connecting to: ");
    Serial.println(savedSSID);

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    status = WIFI_CONNECTING;
    connectStartTime = millis();

    return true;
}

void WiFiManager::startAPMode() {
    Serial.println("[WiFi] Starting Access Point...");

    // Stop any existing connection
    WiFi.disconnect();
    delay(100);

    // Start AP mode
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);

    Serial.print("[WiFi] AP IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Start DNS server for captive portal
    if (!dnsServer) {
        dnsServer = new DNSServer();
    }
    dnsServer->start(53, "*", WiFi.softAPIP());

    // Create captive portal server if not exists
    if (!captiveServer) {
        captiveServer = new AsyncWebServer(80);
        setupRoutes(captiveServer);
        captiveServer->begin();
    }

    status = WIFI_AP_MODE;
    apStartTime = millis();

    Serial.println("[WiFi] Captive Portal started");
    Serial.print("[WiFi] Connect to WiFi: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi] Password: ");
    Serial.println(AP_PASSWORD);
}

void WiFiManager::stopAPMode() {
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }

    if (captiveServer) {
        captiveServer->end();
        delete captiveServer;
        captiveServer = nullptr;
    }

    WiFi.softAPdisconnect(true);
}

bool WiFiManager::shouldRetryConnection() {
    return savedSSID.length() > 0 && status == WIFI_AP_MODE &&
           millis() - apStartTime > CAPTIVE_PORTAL_TIMEOUT;
}

String WiFiManager::getNetworksJSON() {
    int n = WiFi.scanNetworks();
    String json = "[";

    for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encryption\":" + String(WiFi.encryptionType(i));
        json += "}";
    }

    json += "]";
    return json;
}

String WiFiManager::getCaptivePortalHTML() {
    return R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 PID - WiFi Setup</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }
        .container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
            max-width: 500px;
            width: 100%;
            padding: 40px;
        }
        h1 {
            color: #667eea;
            margin-bottom: 10px;
            font-size: 28px;
        }
        .subtitle {
            color: #666;
            margin-bottom: 30px;
            font-size: 14px;
        }
        .form-group {
            margin-bottom: 20px;
        }
        label {
            display: block;
            margin-bottom: 8px;
            color: #333;
            font-weight: 500;
            font-size: 14px;
        }
        input, select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            font-size: 14px;
            transition: border-color 0.3s;
        }
        input:focus, select:focus {
            outline: none;
            border-color: #667eea;
        }
        button {
            width: 100%;
            padding: 14px;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: transform 0.2s, box-shadow 0.2s;
        }
        button:hover {
            transform: translateY(-2px);
            box-shadow: 0 10px 20px rgba(102, 126, 234, 0.4);
        }
        button:active {
            transform: translateY(0);
        }
        .scan-btn {
            background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%);
            margin-bottom: 20px;
        }
        .network-item {
            padding: 12px;
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            margin-bottom: 10px;
            cursor: pointer;
            transition: all 0.3s;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .network-item:hover {
            border-color: #667eea;
            background: #f8f9ff;
        }
        .network-item.selected {
            border-color: #667eea;
            background: #f0f3ff;
        }
        .network-name {
            font-weight: 500;
            color: #333;
        }
        .network-signal {
            color: #666;
            font-size: 12px;
        }
        .networks-list {
            max-height: 300px;
            overflow-y: auto;
            margin-bottom: 20px;
        }
        .loader {
            border: 3px solid #f3f3f3;
            border-top: 3px solid #667eea;
            border-radius: 50%;
            width: 30px;
            height: 30px;
            animation: spin 1s linear infinite;
            margin: 20px auto;
        }
        @keyframes spin {
            0% { transform: rotate(0deg); }
            100% { transform: rotate(360deg); }
        }
        .status {
            padding: 12px;
            border-radius: 8px;
            margin-bottom: 20px;
            font-size: 14px;
        }
        .status.error {
            background: #fee;
            color: #c33;
            border: 2px solid #fcc;
        }
        .status.success {
            background: #efe;
            color: #3c3;
            border: 2px solid #cfc;
        }
        .status.info {
            background: #eef;
            color: #33c;
            border: 2px solid #ccf;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌐 ESP32 PID Controller</h1>
        <p class="subtitle">WiFi Configuration Setup</p>

        <div id="status" style="display:none;"></div>

        <button class="scan-btn" onclick="scanNetworks()">📡 Scan for Networks</button>

        <div id="networks" class="networks-list"></div>

        <form id="wifiForm" onsubmit="saveWiFi(event)">
            <div class="form-group">
                <label for="ssid">Network Name (SSID)</label>
                <input type="text" id="ssid" name="ssid" required placeholder="Enter WiFi SSID">
            </div>

            <div class="form-group">
                <label for="password">Password</label>
                <input type="password" id="password" name="password" placeholder="Enter WiFi password (leave empty if open)">
            </div>

            <button type="submit">💾 Save & Connect</button>
        </form>
    </div>

    <script>
        let selectedNetwork = null;

        function showStatus(message, type) {
            const status = document.getElementById('status');
            status.className = 'status ' + type;
            status.textContent = message;
            status.style.display = 'block';
            setTimeout(() => {
                status.style.display = 'none';
            }, 5000);
        }

        async function scanNetworks() {
            const networksDiv = document.getElementById('networks');
            networksDiv.innerHTML = '<div class="loader"></div>';

            try {
                const response = await fetch('/scan');
                const networks = await response.json();

                if (networks.length === 0) {
                    networksDiv.innerHTML = '<p style="text-align:center;color:#666;">No networks found</p>';
                    return;
                }

                networksDiv.innerHTML = '';
                networks.sort((a, b) => b.rssi - a.rssi);

                networks.forEach(network => {
                    const item = document.createElement('div');
                    item.className = 'network-item';
                    item.onclick = () => selectNetwork(network.ssid, item);

                    const signal = network.rssi;
                    const bars = signal > -50 ? '📶' : signal > -60 ? '📶' : signal > -70 ? '📶' : '📶';
                    const security = network.encryption === 7 ? '🔓' : '🔒';

                    item.innerHTML = `
                        <span class="network-name">${security} ${network.ssid}</span>
                        <span class="network-signal">${bars} ${signal} dBm</span>
                    `;

                    networksDiv.appendChild(item);
                });

                showStatus('Found ' + networks.length + ' networks', 'success');
            } catch (error) {
                networksDiv.innerHTML = '<p style="text-align:center;color:#c33;">Scan failed</p>';
                showStatus('Failed to scan networks', 'error');
            }
        }

        function selectNetwork(ssid, element) {
            document.querySelectorAll('.network-item').forEach(item => {
                item.classList.remove('selected');
            });
            element.classList.add('selected');
            document.getElementById('ssid').value = ssid;
            selectedNetwork = ssid;
            showStatus('Selected: ' + ssid, 'info');
        }

        async function saveWiFi(event) {
            event.preventDefault();

            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;

            if (!ssid) {
                showStatus('Please enter a network name', 'error');
                return;
            }

            showStatus('Saving configuration...', 'info');

            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);

            try {
                const response = await fetch('/save', {
                    method: 'POST',
                    body: formData
                });

                if (response.ok) {
                    showStatus('Configuration saved! Connecting to ' + ssid + '...', 'success');
                    document.getElementById('wifiForm').style.display = 'none';
                    setTimeout(() => {
                        document.body.innerHTML = '<div class="container" style="text-align:center;"><h1>✅ Success!</h1><p>ESP32 is connecting to your network.</p><p>Please reconnect to your WiFi and access the device at its new IP address.</p></div>';
                    }, 2000);
                } else {
                    showStatus('Failed to save configuration', 'error');
                }
            } catch (error) {
                showStatus('Connection error', 'error');
            }
        }

        // Auto-scan on load
        window.onload = () => {
            scanNetworks();
        };
    </script>
</body>
</html>
)";
}
