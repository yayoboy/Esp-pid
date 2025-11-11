# ESP32 PID Controller

Un controller PID professionale basato su ESP32 con supporto per display multipli, sensori di temperatura avanzati, calibrazione, interfaccia web e aggiornamento OTA.

## Caratteristiche

- **Controller PID** completo e configurabile con anti-windup
- **Supporto Display Multipli**:
  - TFT Touch Display 240x320 (ILI9341/ST7789)
  - OLED 0.96" I2C (SSD1315/SSD1306) con tastiera 4x4
- **Sensori di Temperatura Supportati**:
  - DHT22 (Temperatura e Umidità: -40°C a +80°C)
  - BME280 (Temperatura, Umidità, Pressione: -40°C a +85°C)
  - **MAX31855** (Termocoppia tipo K: -200°C a +1350°C) ⚡ NUOVO
  - **MAX6675** (Termocoppia tipo K: 0°C a +1024°C) ⚡ NUOVO
  - **DS18B20** (OneWire waterproof: -55°C a +125°C) ⚡ NUOVO
  - Encoder Rotativo (Posizione)
- **Sistema di Calibrazione Avanzato** ⚡ NUOVO:
  - Calibrazione manuale (offset e scale)
  - Auto-calibrazione con riferimento noto
  - Compensazione errori del sensore
- **Controllo Output Multiplo**:
  - Output PWM per controllo continuo
  - **4 Relé indipendenti** per carichi ON/OFF ⚡ NUOVO
  - Modalità riscaldamento, raffreddamento o dual-mode
  - Time proportioning per controllo fine
- **Interfaccia Web** moderna e responsive per configurazione e monitoraggio
- **OTA Update** (Over-The-Air) per aggiornamenti firmware remoti
- **Configurazione Persistente** su LittleFS

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

### Sensori di Temperatura (scegliere uno o più)
**Sensori Standard**:
- DHT22 (Temperatura/Umidità: -40°C a +80°C)
- BME280 (Temperatura/Umidità/Pressione) via I2C

**Termocoppie per Alte Temperature** ⚡ NUOVO:
- MAX31855 + Termocoppia tipo K (-200°C a +1350°C)
- MAX6675 + Termocoppia tipo K (0°C a +1024°C)

**Sensore Digitale Waterproof** ⚡ NUOVO:
- DS18B20 OneWire (-55°C a +125°C)
- Supporto multi-sensore sullo stesso bus

**Altri Input**:
- Encoder Rotativo (per input posizione)

### Controllo Output
- **PWM**: 1 canale per controllo continuo (ventole, dimmer)
- **Relé**: 4 canali per carichi ON/OFF (riscaldatori, motori, luci)

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

**Termocoppie (MAX31855/MAX6675):** ⚡ NUOVO
```
ESP32          MAX31855      Termocoppia
-----          --------      -----------
GPIO 5    -->  CS
GPIO 18   -->  SCK
GPIO 19   -->  MISO (SO)
3.3V      -->  VCC
GND       -->  GND
              T+ (giallo) <-- Tipo K +
              T- (rosso)  <-- Tipo K -
```

**DS18B20 (OneWire):** ⚡ NUOVO
```
ESP32          DS18B20
-----          -------
GPIO 26   -->  DATA (con pull-up 4.7kΩ a 3.3V)
3.3V      -->  VCC
GND       -->  GND
```

**Relé (4 canali):** ⚡ NUOVO
```
ESP32          Modulo Relé
-----          -----------
GPIO 27   -->  IN1 (Riscaldamento)
GPIO 14   -->  IN2 (Raffreddamento/Ventola)
GPIO 12   -->  IN3 (Allarme/Aux)
GPIO 13   -->  IN4 (Spare)
5V        -->  VCC
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
2. Il sistema rileva automaticamente i sensori collegati
3. Selezione automatica del primo sensore disponibile
4. Connessione al WiFi configurato
5. Se la connessione fallisce, viene creato un Access Point
6. L'indirizzo IP viene mostrato sul display e sulla seriale

**Output Monitor Seriale**:
```
========================================
ESP32 PID Controller
========================================

Initializing sensors...
========================================
[✓] DHT22 sensor detected
[✗] BME280 sensor not found
[✓] MAX31855 thermocouple detected
[✗] MAX6675 thermocouple not found
[✗] DS18B20 sensor not found
[✓] Encoder initialized
========================================
Active Sensor: MAX31855 K-Type
========================================
```

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
- **Pagina Sensori**: Letture sensori (temp, umidità, posizione)
- **Pagina Network**: Stato WiFi e IP

### Calibrazione Sensori ⚡ NUOVO

**Auto-Calibrazione con Riferimento**:
1. Posizionare il sensore in ambiente con temperatura nota (es. 25.0°C)
2. Attendere stabilizzazione (almeno 5 minuti)
3. Avviare via seriale:
```cpp
sensors.startAutoCalibration(25.0);  // Riferimento: 25.0°C
```
4. Sistema raccoglie 100 campioni
5. Calcola e applica offset automaticamente

**Via Web**:
```json
{
  "cmd": "startAutoCalibration",
  "reference": 25.0
}
```

**Calibrazione Manuale**:
```cpp
sensors.setCalibration(+1.5, 1.0);  // Offset +1.5°C, scale 1.0
sensors.enableCalibration(true);
```

### Controllo Relé ⚡ NUOVO

**Modalità Riscaldamento Solo**:
```cpp
relayController.setMode(RELAY_HEATING_ONLY);
relayController.setHysteresis(0.5);  // ±0.5°C
```

**Modalità Dual (Riscaldamento + Raffreddamento)**:
```cpp
relayController.setMode(RELAY_DUAL_MODE);
relayController.setHysteresis(1.0);  // Deadband 1°C
```

**Time Proportioning** (PWM lento):
```cpp
relayController.enableTimeProportioning(true, 10000);  // 10s window
```

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

## Nuove Funzionalità v2.0 ⚡

### Sensori Avanzati
- **Termocoppie MAX31855/MAX6675**: Misure fino a 1350°C per forni e applicazioni industriali
- **DS18B20**: Sensore waterproof con supporto multi-sensore OneWire
- **Rilevamento Automatico**: Il sistema rileva tutti i sensori collegati all'avvio
- **Selezione Dinamica**: Cambio sensore attivo senza riavvio

### Sistema di Calibrazione
- **Auto-Calibrazione**: Sistema automatico con riferimento noto (100 campioni)
- **Calibrazione Manuale**: Offset e scale regolabili
- **Compensazione in Tempo Reale**: Applicata a tutte le letture

### Controllo Relé
- **4 Relé Indipendenti**: Controllo carichi ON/OFF fino a 10A
- **3 Modalità Operative**: Heating-only, Cooling-only, Dual-mode
- **Time Proportioning**: PWM lento per controllo fine senza hardware PWM
- **Protezione Anti-ciclaggio**: Tempo minimo tra switching per proteggere relé e compressori

## Documentazione Completa

📚 **Guide Dettagliate**:
- **[SENSORS.md](docs/SENSORS.md)** - Guida completa sensori (termocoppie, DS18B20, calibrazione)
- **[RELAY.md](docs/RELAY.md)** - Controllo relé e applicazioni pratiche
- **[WIRING.md](docs/WIRING.md)** - Schemi collegamenti hardware dettagliati
- **[EXAMPLES.md](docs/EXAMPLES.md)** - Esempi applicazioni reali
- **[API.md](docs/API.md)** - Documentazione API REST e WebSocket

## Troubleshooting

### Display non funziona

- Verificare i collegamenti
- Per TFT: controllare il driver (ILI9341 o ST7789) in `config.h`
- Per OLED: verificare l'indirizzo I2C (0x3C o 0x3D)

### Sensori non rilevati

- Controllare alimentazione (3.3V, non 5V!)
- Verificare collegamenti I2C (pull-up necessari)
- Controllare log seriale per messaggi di errore
- **Termocoppie**: Verificare polarità (giallo=+, rosso=-)
- **DS18B20**: Resistenza pull-up 4.7kΩ OBBLIGATORIA

### Sensori con letture errate

- **Utilizzare la calibrazione**: Auto-calibrazione con riferimento noto
- **Verificare GND comune**: Tutti i sensori devono condividere GND
- **Termocoppie**: Cavi corti, lontano da interferenze
- **DS18B20**: Limitare lunghezza cavo (< 20m senza schermatura)

### WiFi non si connette

- Verificare SSID e password
- Il sistema creerà un AP se la connessione fallisce
- Connettersi a "ESP32-PID-AP" (password: pid12345)

### OTA non funziona

- Verificare che ESP32 e PC siano sulla stessa rete
- Controllare il nome host: `esp32-pid.local`
- Su Windows, potrebbe essere necessario installare Bonjour

### Relé non scattano

- Verificare alimentazione modulo (5V, 2A minimo)
- Testare comando manuale: `relayController.setRelay(1, true)`
- Controllare che sia impostata una modalità: `setMode(RELAY_HEATING_ONLY)`
- Verificare pin GPIO corretti in `config.h`

## Licenza

MIT License - Sentiti libero di usare e modificare il codice.

## Contributi

Contributi benvenuti! Apri una issue o pull request su GitHub.

## Contatti

Per domande o supporto, apri una issue su GitHub.
