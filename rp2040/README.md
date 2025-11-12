# RP2040 PID Controller

Controller PID completo per Raspberry Pi Pico (RP2040) con configurazione tramite menu display e controllo standalone (senza WiFi).

## Caratteristiche

- **Controller PID** completo e configurabile
- **Interfaccia Menu** navigabile con encoder rotativo e pulsanti
- **Display Multipli**:
  - TFT Touch Display 240x320 (ILI9341/ST7789)
  - OLED 0.96" I2C (SSD1315/SSD1306)
- **Sensori di Temperatura**:
  - DHT22 (Temperatura e Umidità)
  - BME280 (Temperatura, Umidità, Pressione)
  - MAX31855 (Termocoppia K-type: -200°C a +1350°C)
  - MAX6675 (Termocoppia K-type: 0°C a +1024°C)
  - DS18B20 (OneWire: -55°C a +125°C)
- **Sistema di Calibrazione** con auto-calibrazione
- **4 Relé Indipendenti** per carichi ON/OFF
- **Configurazione EEPROM** persistente
- **Standalone** - Non richiede WiFi o PC

## Hardware Richiesto

### Componenti Base
- Raspberry Pi Pico (RP2040)
- Alimentatore 5V (USB o esterno)
- Encoder rotativo con pulsante
- 2 pulsanti aggiuntivi (Enter, Back)

### Display (scegliere uno)
- **TFT 240x320** (ILI9341/ST7789) con touch opzionale
- **OLED 0.96"** (SSD1315/SSD1306)

### Sensori (opzionali)
- DHT22, BME280, MAX31855, MAX6675, o DS18B20

### Output
- PWM per controllo continuo
- 4 canali relé per carichi ON/OFF

## Schema Collegamenti

### Raspberry Pi Pico Pinout

```
┌─────────────────────────────┐
│    Raspberry Pi Pico        │
│                             │
│  GP0  (SPI0 RX)  ──────────┤ Thermo MISO
│  GP1             ──────────┤ Thermo CS
│  GP2  (DHT/SCK)  ──────────┤ DHT22 / Thermo SCK
│  GP3             ──────────┤ DS18B20
│  GP4  (I2C0 SDA) ──────────┤ BME280/OLED SDA
│  GP5  (I2C0 SCL) ──────────┤ BME280/OLED SCL
│  GP6             ──────────┤ Encoder A
│  GP7             ──────────┤ Encoder B
│  GP8             ──────────┤ Encoder Button
│  GP9             ──────────┤ Back Button
│  GP10            ──────────┤ Up Button
│  GP11            ──────────┤ Down Button
│  GP12            ──────────┤ Position Encoder A
│  GP13            ──────────┤ Position Encoder B
│  GP14            ──────────┤ Touch CS
│  GP15            ──────────┤ TFT RST
│  GP16            ──────────┤ TFT DC
│  GP17            ──────────┤ TFT CS
│  GP18 (SPI1 SCK) ──────────┤ TFT SCLK
│  GP19 (SPI1 TX)  ──────────┤ TFT MOSI
│  GP20 (SPI1 RX)  ──────────┤ TFT MISO
│  GP21            ──────────┤ PWM Output
│  GP22            ──────────┤ Relay 1 (Heat)
│  GP26            ──────────┤ Relay 2 (Cool)
│  GP27            ──────────┤ Relay 3 (Alarm)
│  GP28            ──────────┤ Relay 4 (Spare)
│  LED (GP25)      ──────────┤ Status LED
│                             │
│  3V3 OUT         ──────────┤ → Sensori 3.3V
│  GND             ──────────┤ → GND comune
│  VSYS            ──────────┤ → 5V in (se alimentato esternamente)
└─────────────────────────────┘
```

### Input Controls

```
Encoder Rotativo:
GP6 ──> Encoder A (CLK)
GP7 ──> Encoder B (DT)
GP8 ──> Encoder SW (pulsante integrato)
GND ──> GND

Pulsanti:
GP9  ──> Back Button  ──> GND (con pull-up interno)
GP10 ──> Up Button    ──> GND (opzionale)
GP11 ──> Down Button  ──> GND (opzionale)
```

### TFT Display (SPI1)

```
Pico          TFT Display
----          -----------
GP17     -->  CS
GP16     -->  DC
GP15     -->  RST
GP19     -->  MOSI (SDA)
GP18     -->  SCK (CLK)
GP20     -->  MISO
3.3V     -->  VCC
GND      -->  GND

Touch (opzionale):
GP14     -->  T_CS
GP13     -->  T_IRQ
```

### OLED Display (I2C0)

```
Pico          OLED
----          ----
GP4      -->  SDA
GP5      -->  SCL
3.3V     -->  VCC
GND      -->  GND
```

### Sensori

**DHT22:**
```
GP2  --> DHT22 DATA
3.3V --> VCC
GND  --> GND
```

**BME280 (I2C):**
```
GP4  --> SDA
GP5  --> SCL
3.3V --> VCC
GND  --> GND
```

**MAX31855/MAX6675 (SPI0):**
```
GP1  --> CS
GP2  --> SCK (shared)
GP0  --> MISO (shared)
3.3V --> VCC
GND  --> GND

Termocoppia K-type:
T+ (giallo) --> MAX31855 T+
T- (rosso)  --> MAX31855 T-
```

**DS18B20 (OneWire):**
```
GP3  --> DATA (con pull-up 4.7kΩ a 3.3V)
3.3V --> VCC
GND  --> GND
```

### Output

**PWM:**
```
GP21 --> Driver/MOSFET
```

**Relé:**
```
GP22 --> Relay Module IN1 (Heating)
GP26 --> Relay Module IN2 (Cooling)
GP27 --> Relay Module IN3 (Alarm)
GP28 --> Relay Module IN4 (Spare)
5V   --> Relay Module VCC
GND  --> Relay Module GND
```

## Configurazione Software

### PlatformIO

Usa il file `platformio_rp2040.ini`:

```bash
# Compila
pio run -c platformio_rp2040.ini

# Upload
pio run -c platformio_rp2040.ini --target upload

# Monitor
pio device monitor
```

### Selezione Display

Modifica `rp2040/include/config.h`:

```cpp
// Per display TFT
#define USE_TFT_DISPLAY

// Per display OLED (commentare TFT)
// #define USE_OLED_DISPLAY
```

## Utilizzo

### Primo Avvio

1. Alimentare il Pico via USB o 5V
2. Il sistema rileva i sensori automaticamente
3. Display mostra schermata principale con:
   - Setpoint
   - Temperatura corrente
   - Output PID
   - Modalità (AUTO/MANUAL)

### Navigazione Menu

**Schermata Principale:**
- **Ruota Encoder**: Regola setpoint
- **Click Encoder**: Toggle AUTO/MANUAL
- **Tieni Encoder (1s)**: Entra in menu
- **Pulsante Back**: Entra in menu (alternativo)

**All'interno del Menu:**
- **Ruota Encoder**: Naviga tra opzioni
- **Click Encoder**: Seleziona/Conferma
- **Pulsante Back**: Torna indietro
- **Timeout 30s**: Ritorna automaticamente alla schermata principale

### Struttura Menu

```
Main Menu
├── PID Settings
│   ├── Mode (AUTO/MANUAL)
│   ├── Setpoint
│   ├── Kp
│   ├── Ki
│   ├── Kd
│   └── Reset PID
├── Sensor Select
│   ├── DHT22
│   ├── BME280
│   ├── MAX31855
│   ├── MAX6675
│   ├── DS18B20
│   └── Back
├── Calibration
│   ├── Offset
│   ├── Scale
│   ├── Enabled (ON/OFF)
│   └── Auto-Cal
├── Relay Settings
│   ├── Mode (OFF/HEAT/COOL/DUAL)
│   ├── Hysteresis
│   ├── Time Proportioning
│   └── Manual Control
├── System Info
│   ├── Device Name
│   ├── Firmware Version
│   ├── Current Temp
│   ├── Uptime
│   ├── Free RAM
│   └── Active Sensor
├── Save/Load
│   ├── Save Config
│   ├── Load Config
│   ├── Reset to Default
│   └── Reboot System
└── Exit Menu
```

### Modifica Valori

Quando si modifica un valore (Setpoint, Kp, etc.):

1. Seleziona l'opzione e premi Encoder
2. Il valore lampeggia (modalità edit)
3. Ruota Encoder per modificare
4. Click Encoder per confermare
5. Pulsante Back per annullare

### Calibrazione

**Calibrazione Manuale:**
1. Menu → Calibration
2. Seleziona "Offset" o "Scale"
3. Modifica il valore
4. Abilita "Enabled" → ON

**Auto-Calibrazione:**
1. Posiziona sensore in ambiente con temperatura nota
2. Attendi stabilizzazione (5+ minuti)
3. Menu → Calibration → Auto-Cal
4. Inserisci temperatura di riferimento (es. 25.0°C)
5. Sistema raccoglie 100 campioni e calcola offset

### Salvataggio Configurazione

**Automatico:**
- Ogni modifica viene salvata in EEPROM automaticamente

**Manuale:**
- Menu → Save/Load → Save Config

**Ripristino Default:**
- Menu → Save/Load → Reset to Default

## Status LED

Il LED integrato (GP25) indica lo stato:

- **Lampeggio lento**: Modalità AUTO (PID attivo)
- **Acceso fisso**: Modalità MANUAL
- **Spento**: Sistema non operativo

## Differenze con Versione ESP32

| Caratteristica | ESP32 | RP2040 |
|----------------|-------|--------|
| WiFi/Web | ✅ Sì | ❌ No |
| OTA Update | ✅ Sì | ❌ No |
| Configurazione | Web | Menu Display |
| Storage | LittleFS | EEPROM |
| Costo | ~€5-10 | ~€4 |
| Consumo | ~240mA | ~50mA |
| GPIO | 34 | 26 |
| RAM | 520KB | 264KB |
| Flash | 4MB | 2MB |

## Vantaggi RP2040

✅ **Standalone** - Funziona senza PC o rete
✅ **Basso consumo** - Ideale per batteria
✅ **Costo ridotto** - €4 per Pico
✅ **Dual core** - RP2040 ha 2 core ARM Cortex-M0+
✅ **PIO** - Periferiche programmabili uniche
✅ **Semplicità** - No configurazione WiFi necessaria

## Applicazioni Ideali

- **Incubatrici** - Controllo temperatura/umidità standalone
- **Fermentatori** - Dual-mode heating/cooling
- **Data Logger** - Registrazione locale (SD card addon)
- **Controlli industriali** - Installazioni remote senza rete
- **Prototyping** - Test rapido senza setup web

## Troubleshooting

### Display non funziona
- Verificare SPI: GP18 (SCK), GP19 (MOSI) per TFT
- Verificare I2C: GP4 (SDA), GP5 (SCL) per OLED
- Controllare alimentazione 3.3V

### Encoder non risponde
- Verificare collegamenti GP6, GP7, GP8
- Controllare resistenze pull-up interne (abilitate di default)
- Regolare ENCODER_SENSITIVITY in config.h

### Configurazione non si salva
- EEPROM richiede chiamata `EEPROM.commit()` (già implementato)
- Verificare con Menu → System Info

### Sensori non rilevati
- Controllare alimentazione 3.3V
- Verificare GND comune
- DS18B20: Resistenza pull-up 4.7kΩ OBBLIGATORIA

## Upgrade Futuri

Possibili miglioramenti:
- [ ] Supporto SD card per data logging
- [ ] LCD 20x4 con I2C
- [ ] Supporto RTC per timestamp
- [ ] Profili temperatura programmabili
- [ ] Bluetooth Low Energy (opzionale)

## File Sorgenti

```
rp2040/
├── include/
│   ├── config.h                   # Configurazione pin e parametri
│   ├── MenuSystem.h               # Sistema menu
│   ├── InputHandler.h             # Gestione encoder/pulsanti
│   └── ConfigManager_RP2040.h     # Storage EEPROM
├── src/
│   ├── main.cpp                   # Loop principale
│   ├── MenuSystem.cpp
│   ├── InputHandler.cpp
│   └── ConfigManager_RP2040.cpp
└── README.md
```

**File Condivisi con ESP32:**
- PIDController.h/cpp
- SensorManager.h/cpp (con adattamenti pin)
- RelayController.h/cpp
- DisplayManager.h/cpp

## Compilazione

```bash
# Clone repository
git clone <repo-url>
cd Esp-pid

# Compila versione RP2040
pio run -c platformio_rp2040.ini

# Upload (Pico in modalità BOOTSEL)
# 1. Tieni premuto BOOTSEL
# 2. Collega USB
# 3. Rilascia BOOTSEL
pio run -c platformio_rp2040.ini --target upload

# Oppure copia manualmente
# File .uf2 in .pio/build/pico/
# Copia su drive RPI-RP2 che appare
```

## Licenza

MIT License - Stesso della versione ESP32

## Supporto

Per problemi o domande:
- Versione ESP32: Vedi README principale
- Versione RP2040: Apri issue su GitHub con tag [RP2040]

## Contributi

Contributi benvenuti! Aree di interesse:
- Ottimizzazione menu UI
- Nuovi tipi di sensori
- Supporto display aggiuntivi
- Data logging su SD

---

**Nota**: Questa è una versione standalone completamente funzionale del controller PID. Per la versione con WiFi e interfaccia web, usa la versione ESP32 (vedi README principale).
