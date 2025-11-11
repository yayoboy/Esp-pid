# ESP32 PID Controller

Un controller PID completo basato su ESP32 con supporto per display multipli, vari sensori, interfaccia web e aggiornamento OTA.

## Caratteristiche

- **Controller PID** completo e configurabile
- **Supporto Display Multipli**:
  - TFT Touch Display 240x320 (ILI9341/ST7789)
  - OLED 0.96" I2C (SSD1315) con tastiera 4x4
- **Sensori Supportati**:
  - DHT22 (Temperatura e Umidità)
  - BME280 (Temperatura, Umidità, Pressione)
  - Encoder Rotativo (Posizione)
- **Interfaccia Web** moderna e responsive per configurazione e monitoraggio
- **OTA Update** (Over-The-Air) per aggiornamenti firmware remoti
- **Configurazione Persistente** su LittleFS
- **Output PWM** per controllo attuatori

## Hardware Richiesto

### Componenti Base
- ESP32 Development Board
- Alimentatore 5V (USB o esterno)

### Display (scegliere uno)
**Opzione 1: TFT Touch Display**
- Display TFT 240x320 con driver ILI9341 o ST7789
- Touch controller integrato

**Opzione 2: OLED Display**
- Display OLED 0.96" I2C (SSD1315/SSD1306)
- Tastiera 4x4 opzionale

### Sensori (opzionali)
- DHT22 (Temperatura/Umidità)
- BME280 (Temperatura/Umidità/Pressione) via I2C
- Encoder Rotativo (per input posizione)

## Schema Collegamenti

### Opzione 1: TFT Display (ILI9341/ST7789)

```
ESP32          TFT Display
-----          -----------
GPIO 15   -->  CS
GPIO 2    -->  DC
GPIO 4    -->  RST
GPIO 23   -->  MOSI (SDA)
GPIO 18   -->  SCK (CLK)
GPIO 19   -->  MISO
3.3V      -->  VCC
GND       -->  GND
GPIO 5    -->  T_CS (Touch)
GPIO 17   -->  T_IRQ (Touch)
```

### Opzione 2: OLED Display (SSD1315)

```
ESP32          OLED Display
-----          ------------
GPIO 21   -->  SDA
GPIO 22   -->  SCL
3.3V      -->  VCC
GND       -->  GND
```

### Sensori

**DHT22:**
```
ESP32          DHT22
-----          -----
GPIO 16   -->  DATA
3.3V      -->  VCC
GND       -->  GND
```

**BME280 (I2C):**
```
ESP32          BME280
-----          ------
GPIO 21   -->  SDA
GPIO 22   -->  SCL
3.3V      -->  VCC
GND       -->  GND
```

**Encoder Rotativo:**
```
ESP32          Encoder
-----          -------
GPIO 34   -->  CLK (A)
GPIO 35   -->  DT (B)
GND       -->  GND
```

**Output PWM:**
```
ESP32          Attuatore
-----          ---------
GPIO 25   -->  Control Input
GND       -->  GND
```

## Configurazione Software

### 1. Selezione Display

Modificare il file `include/config.h` per selezionare il display:

```cpp
// Per display TFT
#define USE_TFT_DISPLAY

// Per display OLED
// #define USE_OLED_DISPLAY
```

### 2. Configurazione WiFi

Modificare le credenziali WiFi in `include/config.h`:

```cpp
#define WIFI_SSID "YourSSID"
#define WIFI_PASSWORD "YourPassword"
#define HOSTNAME "esp32-pid"
```

### 3. Pin Personalizzati

Tutti i pin sono configurabili nel file `include/config.h`.

## Compilazione e Upload

### PlatformIO (Raccomandato)

```bash
# Installa PlatformIO CLI
pip install platformio

# Compila il progetto
pio run

# Upload via USB
pio run --target upload

# Upload via OTA (dopo il primo upload)
pio run --target upload --upload-port esp32-pid.local

# Monitor seriale
pio device monitor
```

### Arduino IDE

1. Installa le librerie richieste (vedi `platformio.ini`)
2. Apri `src/main.cpp`
3. Seleziona Board: "ESP32 Dev Module"
4. Compila e carica

## Utilizzo

### Primo Avvio

1. Alimentare l'ESP32
2. Il sistema si connette al WiFi configurato
3. Se la connessione fallisce, viene creato un Access Point
4. L'indirizzo IP viene mostrato sul display e sulla seriale

### Interfaccia Web

Aprire un browser e navigare all'indirizzo IP dell'ESP32:

```
http://[IP_ADDRESS]
```

L'interfaccia web permette di:
- Monitorare valori in tempo reale
- Configurare parametri PID (Kp, Ki, Kd)
- Impostare il setpoint
- Cambiare modalità (Auto/Manuale)
- Visualizzare dati sensori

### Display Locale

Il display mostra:
- **Pagina Principale**: Setpoint, valore corrente, output, modalità
- **Pagina Tuning**: Parametri PID
- **Pagina Sensori**: Letture sensori
- **Pagina Network**: Stato WiFi e IP

### OTA Update

Per aggiornare il firmware via OTA:

```bash
# Via PlatformIO
pio run -e esp32dev_ota --target upload

# Via Arduino IDE
Tools -> Port -> esp32-pid.local
```

Password OTA default: `admin`

## Parametri PID

### Valori Default

- **Kp (Proporzionale)**: 2.0
- **Ki (Integrale)**: 5.0
- **Kd (Derivativo)**: 1.0
- **Setpoint**: 25.0

### Tuning

Per tuning ottimale:

1. Iniziare con Ki=0 e Kd=0
2. Aumentare Kp fino a oscillazioni
3. Ridurre Kp del 50%
4. Aumentare Ki per eliminare errore stazionario
5. Aggiungere Kd per ridurre overshoot

## API REST

### Endpoints Disponibili

**GET /api/status**
```json
{
  "setpoint": 25.0,
  "input": 24.5,
  "output": 128,
  "mode": true,
  "kp": 2.0,
  "ki": 5.0,
  "kd": 1.0,
  "temp": 24.5,
  "humidity": 45.2,
  "position": 100
}
```

**GET /api/config**
```json
{
  "kp": 2.0,
  "ki": 5.0,
  "kd": 1.0,
  "setpoint": 25.0
}
```

### WebSocket

Connessione: `ws://[IP_ADDRESS]/ws`

**Messaggi in invio:**
```json
// Impostare PID
{"cmd": "setPID", "kp": 2.0, "ki": 5.0, "kd": 1.0}

// Impostare setpoint
{"cmd": "setSetpoint", "value": 25.0}

// Cambiare modalità
{"cmd": "setMode", "value": true}
```

**Messaggi in ricezione:**
```json
{
  "type": "status",
  "setpoint": 25.0,
  "input": 24.5,
  "output": 128,
  ...
}
```

## File System

I dati di configurazione sono salvati in `/config.json` su LittleFS:

```json
{
  "pid": {
    "kp": 2.0,
    "ki": 5.0,
    "kd": 1.0,
    "setpoint": 25.0,
    "autoMode": true
  },
  "wifi": {
    "ssid": "MySSID",
    "password": "MyPassword",
    "hostname": "esp32-pid"
  }
}
```

## Troubleshooting

### Display non funziona

- Verificare i collegamenti
- Per TFT: controllare il driver (ILI9341 o ST7789) in `config.h`
- Per OLED: verificare l'indirizzo I2C (0x3C o 0x3D)

### Sensori non rilevati

- Controllare alimentazione (3.3V, non 5V!)
- Verificare collegamenti I2C (pull-up necessari)
- Controllare log seriale per messaggi di errore

### WiFi non si connette

- Verificare SSID e password
- Il sistema creerà un AP se la connessione fallisce
- Connettersi a "ESP32-PID-AP" (password: pid12345)

### OTA non funziona

- Verificare che ESP32 e PC siano sulla stessa rete
- Controllare il nome host: `esp32-pid.local`
- Su Windows, potrebbe essere necessario installare Bonjour

## Licenza

MIT License - Sentiti libero di usare e modificare il codice.

## Contributi

Contributi benvenuti! Apri una issue o pull request su GitHub.

## Contatti

Per domande o supporto, apri una issue su GitHub.
