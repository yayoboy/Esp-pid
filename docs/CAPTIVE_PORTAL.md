# WiFi Captive Portal

The ESP32 PID Controller features an automatic captive portal system for easy WiFi configuration without hardcoding credentials or using serial commands.

## Features

- **Automatic Activation**: Captive portal starts automatically on first boot or when WiFi connection fails
- **User-Friendly Interface**: Modern, mobile-friendly web interface for WiFi setup
- **Network Scanning**: Automatically scans and displays available WiFi networks
- **Credential Storage**: Saves WiFi credentials persistently in LittleFS
- **Auto-Reconnection**: Automatically reconnects to saved network after timeout
- **Reset Mechanism**: Hardware button to reset WiFi credentials
- **Status Indicators**: Clear status display and LED indicators

## How It Works

### First Boot Sequence

1. **System Startup**: ESP32 boots and initializes all components
2. **Check Credentials**: WiFiManager checks for saved WiFi credentials
3. **No Credentials Found**: If no valid credentials exist, captive portal starts automatically
4. **AP Mode Active**: ESP32 creates its own WiFi access point

### Access Point Details

When in AP mode, the ESP32 creates a WiFi network with these credentials:

- **SSID**: `ESP32-PID-Setup`
- **Password**: `setup123`
- **IP Address**: `192.168.4.1`

### Configuration Process

1. **Connect to AP**:
   - Use your phone/computer to connect to `ESP32-PID-Setup`
   - Enter password: `setup123`

2. **Automatic Redirect**:
   - Most devices will automatically show a "Sign in to network" prompt
   - This opens the captive portal configuration page
   - If not redirected, manually navigate to: `http://192.168.4.1`

3. **Scan Networks**:
   - The page automatically scans for available WiFi networks
   - Networks are displayed sorted by signal strength
   - Shows signal strength (RSSI) and security type (🔒/🔓)

4. **Select Network**:
   - Click/tap on your desired network to select it
   - Or manually enter SSID if your network is hidden
   - Enter the WiFi password (leave empty for open networks)

5. **Save & Connect**:
   - Click "Save & Connect" button
   - ESP32 saves credentials and restarts
   - After restart, ESP32 connects to your WiFi network

6. **Access Device**:
   - Once connected, the ESP32 gets an IP from your router
   - Check serial monitor for the new IP address
   - Access web interface at: `http://<esp32-ip-address>`

## Visual Interface

**Important Note**: The captive portal is a web interface accessed via browser on your smartphone, tablet, or computer - NOT on the ESP32's physical display. The ESP32 display (TFT touch or OLED with keypad) shows only status information (WiFi status, IP address, etc.). Configuration is done through your device's web browser.

The captive portal web interface features a modern, gradient design with:

- **Network List**: Shows all available networks with signal strength
- **Visual Feedback**: Selected network is highlighted
- **Real-time Status**: Shows scan progress, errors, and success messages
- **Responsive Design**: Works on phones, tablets, and computers
- **Touch-Friendly**: Large tap targets for mobile devices

## Captive Portal Behavior

### Auto-Retry Logic

If WiFi connection fails or credentials are incorrect:

1. ESP32 attempts connection for 20 seconds (configurable)
2. If failed, captive portal starts automatically
3. Portal stays active for 5 minutes (configurable timeout)
4. After timeout, ESP32 retries saved credentials
5. Cycle repeats until successful connection

### During Operation

- **Connection Lost**: If WiFi drops during operation, ESP32 attempts automatic reconnection
- **Failed Reconnection**: After multiple failures, captive portal restarts
- **AP Mode Indicator**: Status LED shows different patterns for AP mode vs connected mode

## Reset WiFi Credentials

There are two ways to reset WiFi credentials and restart the captive portal:

### Method 1: Hardware Button (Recommended)

1. **Hold BOOT Button**: Press and hold the BOOT button (GPIO0) on ESP32
2. **Wait 5 Seconds**: Keep holding for at least 5 seconds
3. **Release**: Release button - credentials are now cleared
4. **Captive Portal Starts**: ESP32 automatically starts captive portal

### Method 2: Web API

Send POST request to `/reset` endpoint:

```bash
curl -X POST http://<esp32-ip>/reset
```

Or use the web interface if available.

### Method 3: Factory Reset (Complete)

Delete the configuration file from serial monitor:

```bash
# Using PlatformIO
pio run -t erase
```

This erases all configuration including PID settings and pin assignments.

## Configuration Constants

You can customize captive portal behavior in `include/config.h`:

```cpp
// WiFi Configuration
#define AP_SSID "ESP32-PID-Setup"       // Change AP network name
#define AP_PASSWORD "setup123"           // Change AP password (min 8 chars)
#define WIFI_CONNECT_TIMEOUT 20000       // 20 seconds - connection timeout
#define CAPTIVE_PORTAL_TIMEOUT 300000    // 5 minutes - AP mode timeout
#define WIFI_RESET_PIN 0                 // GPIO0 - BOOT button for reset
```

## Troubleshooting

### Captive Portal Not Opening

**Symptoms**: Connected to AP but portal doesn't open automatically

**Solutions**:
- Manually navigate to `http://192.168.4.1`
- Disable mobile data/cellular connection
- Try a different browser
- Check "Sign in to network" notifications on your device

### Can't Connect to AP

**Symptoms**: ESP32-PID-Setup network not visible

**Solutions**:
- Verify ESP32 is powered on and booted (check serial monitor)
- Wait 30 seconds after power-on for full initialization
- Check if another device is already connected (ESP32 AP has connection limit)
- Restart ESP32

### Wrong Password Error

**Symptoms**: "Authentication error" when connecting to AP

**Solutions**:
- Verify password is exactly: `setup123` (case-sensitive)
- If changed in config.h, use your custom password
- Ensure password is at least 8 characters

### Connection to My Network Fails

**Symptoms**: Portal accepts credentials but connection fails

**Solutions**:
- Verify password is correct (WPA/WPA2)
- Check WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
- Ensure network uses compatible security (WPA/WPA2, not WPA3-only)
- Check if MAC filtering is enabled on router
- Try manually entering SSID for hidden networks

### Portal Keeps Restarting

**Symptoms**: Captive portal appears repeatedly

**Solutions**:
- This is expected if no valid network is configured
- Configure WiFi credentials properly
- Check if saved network is in range
- Verify router is working and broadcasting SSID

### Serial Monitor Shows Errors

**Common Error Messages**:

- `Failed to connect to WiFi, starting AP mode`
  - Normal behavior when no network configured or connection failed
  - Configure WiFi via captive portal

- `Connection timeout - starting captive portal`
  - Saved credentials are incorrect or network is out of range
  - Reset credentials and reconfigure

- `DNS server start failed`
  - Rare - try restarting ESP32
  - Check if another service is using port 53

## Advanced: Direct Configuration

For advanced users, you can directly edit the configuration file:

1. Connect via serial monitor
2. Use LittleFS upload tool to modify `/config.json`
3. Add/modify WiFi credentials:

```json
{
  "wifi": {
    "ssid": "YourNetworkName",
    "password": "YourPassword",
    "hostname": "esp32-pid"
  }
}
```

4. Restart ESP32

## Security Considerations

### AP Mode Security

- **Change Default Password**: Edit `AP_PASSWORD` in config.h before deployment
- **Limited Exposure**: AP mode only active during setup or connection failures
- **Auto-Timeout**: AP mode automatically closes after successful connection
- **Local Only**: Captive portal is not accessible from internet

### Credential Storage

- Credentials stored in LittleFS (flash memory)
- Not encrypted by default (ESP32 limitation)
- Accessible only via serial connection or physical access
- Consider enabling ESP32 flash encryption for sensitive deployments

### Network Security

- Only connects to WPA/WPA2 secured networks (configurable)
- Does not store credentials in code (git-safe)
- Reset button prevents permanent lockout

## API Endpoints

The captive portal exposes these endpoints:

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Main captive portal page |
| `/scan` | GET | Returns JSON list of available networks |
| `/save` | POST | Save WiFi credentials (params: ssid, password) |
| `/reset` | POST | Reset WiFi credentials |
| `/generate_204` | GET | Android captive portal detection |
| `/fwlink` | GET | Windows captive portal detection |

### Example: Save Credentials via API

```bash
curl -X POST http://192.168.4.1/save \
  -d "ssid=MyNetwork" \
  -d "password=MyPassword123"
```

### Example: Scan Networks via API

```bash
curl http://192.168.4.1/scan

# Response:
# [
#   {"ssid":"MyNetwork","rssi":-45,"encryption":3},
#   {"ssid":"Neighbor","rssi":-67,"encryption":3}
# ]
```

## Integration with Other Components

### Display Integration

The ESP32's physical display (TFT or OLED) shows WiFi status information only:
- "WiFi Setup..." during captive portal initialization
- "Connected" with IP address when connected to network
- "AP Mode" when in setup mode with AP IP address (192.168.4.1)

**Note**: The physical display does NOT provide WiFi configuration capabilities. Whether you have:
- **TFT Touch Display (240x320)**: Touch input is used for PID control, not WiFi setup
- **OLED Display (0.96") + 4x4 Keypad**: Buttons are used for local navigation, not WiFi setup

WiFi configuration is done exclusively through the web browser interface on a separate device (smartphone, tablet, or computer) that connects to the ESP32's Access Point.

### Web Interface Integration

- Main web interface is available in both STA and AP modes
- Configuration pages accessible from any mode
- OTA updates only available when connected to WiFi (not in AP mode)

### OTA Integration

- OTA updates disabled in AP mode (for safety)
- Automatically enabled once connected to WiFi
- Update via: `http://<esp32-ip>/update`

## Customization

### Custom Portal HTML

Edit `WiFiManager.cpp`, method `getCaptivePortalHTML()` to customize:

- Logo and branding
- Colors and styling
- Additional configuration options
- Custom JavaScript behavior

### Custom Behavior

Modify `WiFiManager.cpp` to add:

- Email notification on configuration
- SMS alerts
- MQTT publish on WiFi events
- Custom validation rules
- Multiple network profiles

## Technical Details

### Libraries Used

- **DNSServer**: Handles DNS requests for captive portal redirect
- **ESPAsyncWebServer**: Provides HTTP server for portal page
- **WiFi**: ESP32 WiFi driver for STA and AP modes

### State Machine

```
[WIFI_IDLE] --> [WIFI_CONNECTING] --> [WIFI_CONNECTED]
     |                 |                      |
     |                 v                      |
     |          [Connection                  |
     |           Timeout]                    |
     |                 |                      v
     +-------> [WIFI_AP_MODE] <-----[Connection Lost]
                      |
                      v
              [Timeout or Reset]
                      |
                      v
              [Retry Connection]
```

### Memory Usage

- WiFiManager object: ~200 bytes
- Captive portal HTML: ~7 KB (stored in flash)
- DNS server: ~500 bytes
- Web server: ~2 KB
- **Total Flash**: ~10 KB
- **Total RAM**: ~3 KB

### Performance

- Network scan: 1-3 seconds
- Connection attempt: max 20 seconds
- Page load: <500ms (local network)
- DNS response: <10ms

## Future Enhancements

Potential improvements for future versions:

- [ ] Multiple WiFi profile support
- [ ] WPS push-button configuration
- [ ] SmartConfig / ESP-Touch support
- [ ] BLE configuration (via smartphone app)
- [ ] WPA3 support
- [ ] Mesh networking support
- [ ] Guest network isolation
- [ ] VPN configuration

## Related Documentation

- [Main README](../README.md) - General project documentation
- [Web Interface API](../README.md#web-interface) - Web API documentation
- [OTA Updates](../README.md#ota-updates) - Over-the-air update guide
- [Configuration](../README.md#configuration) - General configuration guide

## Support

For issues, questions, or contributions:

- GitHub Issues: [ESP32-PID Issues](https://github.com/yourusername/esp32-pid/issues)
- Serial Debug: Enable debug output in `config.h` with `#define DEBUG_LEVEL 3`
- Community Forum: ESP32 Arduino Forum

---

**Last Updated**: 2024-01-12
**Version**: 1.0.0
**Author**: ESP32 PID Controller Project
