#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include "ConfigManager.h"

enum WiFiStatus {
    WIFI_IDLE,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_AP_MODE,
    WIFI_FAILED
};

class WiFiManager {
public:
    WiFiManager(ConfigManager* configMgr);
    ~WiFiManager();

    // Initialize and start WiFi connection
    bool begin();

    // Update loop - must be called regularly
    void update();

    // Get current WiFi status
    WiFiStatus getStatus() const { return status; }
    String getStatusString() const;

    // Get IP address (either STA or AP mode)
    String getIPAddress() const;

    // Force AP mode with captive portal
    void startCaptivePortal();

    // Reset WiFi credentials and restart captive portal
    void resetCredentials();

    // Check if reset button is being held
    void checkResetButton();

    // Setup web server routes for captive portal
    void setupRoutes(AsyncWebServer* server);

    // Save WiFi credentials
    bool saveCredentials(const String& ssid, const String& password);

private:
    ConfigManager* config;
    DNSServer* dnsServer;
    AsyncWebServer* captiveServer;

    WiFiStatus status;
    unsigned long connectStartTime;
    unsigned long apStartTime;
    unsigned long resetButtonPressTime;
    bool resetButtonPressed;

    String savedSSID;
    String savedPassword;

    // Try to connect to saved WiFi network
    bool connectToWiFi();

    // Start AP mode with captive portal
    void startAPMode();

    // Stop AP mode
    void stopAPMode();

    // Check if we should retry connection
    bool shouldRetryConnection();

    // Captive portal HTML page
    String getCaptivePortalHTML();

    // WiFi scan for available networks
    String getNetworksJSON();
};

#endif // WIFIMANAGER_H
