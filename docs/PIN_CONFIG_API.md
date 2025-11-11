# Pin Configuration API

Documentazione per la configurazione dinamica dei pin via web interface.

## Caratteristiche

- **Configurazione completa dei pin** via interfaccia web
- **Validazione ESP32**: Prevenzione uso pin riservati (6-11, Flash)
- **Pin Input-Only**: Supporto GPIO 34-39 per encoder e sensori
- **Persistenza**: Salvataggio su LittleFS
- **Hot-reload**: Riavvio automatico dopo modifica

## Endpoints API

### GET `/api/pins`

Ottiene la configurazione attuale dei pin.

**Response**:
```json
{
  "dht": 16,
  "bme_sda": 21,
  "bme_scl": 22,
  "encoder_a": 34,
  "encoder_b": 35,
  "thermo_cs": 5,
  "thermo_sck": 18,
  "thermo_miso": 19,
  "ds18b20": 26,
  "pwm": 25,
  "relay_1": 27,
  "relay_2": 14,
  "relay_3": 12,
  "relay_4": 13,
  "tft_cs": 15,
  "tft_dc": 2,
  "tft_rst": 4,
  "tft_mosi": 23,
  "tft_sclk": 18,
  "tft_miso": 19,
  "oled_sda": 21,
  "oled_scl": 22
}
```

### POST `/api/pins`

Aggiorna la configurazione dei pin.

**Request Body**:
```json
{
  "dht": 16,
  "bme_sda": 21,
  ...
}
```

**Response**:
```json
{
  "success": true,
  "message": "Pin configuration saved. Restart required."
}
```

**Errori**:
```json
{
  "success": false,
  "error": "Invalid pin: GPIO 7 is reserved"
}
```

### POST `/api/restart`

Riavvia l'ESP32 dopo modifica configurazione.

**Response**:
```json
{
  "success": true,
  "message": "Restarting..."
}
```

### GET `/pinconfig`

Serve la pagina HTML di configurazione pin.

## Web Interface

Accedere a: `http://[ESP32_IP]/pinconfig`

### Funzionalità UI

1. **Visualizzazione pin correnti**
2. **Modifica con validazione in tempo reale**
3. **Guida pin ESP32 integrata**
4. **Evidenziazione pin riservati**
5. **Salvataggio e riavvio automatico**

### Validazione

#### Pin Validi
- GPIO 0-5, 12-19, 21-23, 25-27, 32-33: Uso generale
- GPIO 34-39: Solo input (encoder, sensori)

#### Pin Non Validi
- ❌ GPIO 6-11: Riservati per Flash
- ⚠️ GPIO 1, 3: TX/RX (evitare)
- ❌ GPIO > 39: Non esistono

### Esempi Configurazione

**Configurazione Standard**:
```
Sensori:
- DHT22: GPIO 16
- BME280: SDA=21, SCL=22
- Thermocouple: CS=5, SCK=18, MISO=19
- DS18B20: GPIO 26
- Encoder: A=34, B=35

Output:
- PWM: GPIO 25
- Relay 1-4: GPIO 27, 14, 12, 13

Display TFT:
- CS=15, DC=2, RST=4
- MOSI=23, SCK=18, MISO=19

Display OLED:
- SDA=21, SCL=22
```

**Configurazione Custom**:
```
Esempio per hardware diverso:
- Encoder su GPIO 36, 39 (input-only)
- Relay su GPIO 32, 33, 25, 26
- DS18B20 su GPIO 4
```

## Utilizzo via Codice

### Leggere Configurazione

```cpp
PinConfig pins = configManager.getPinConfig();

// Usare i pin
pinMode(pins.relay_1, OUTPUT);
digitalWrite(pins.relay_1, HIGH);

// Sensori
DHT dht(pins.dht_pin, DHT22);
```

### Modificare Singolo Pin

```cpp
// Via seriale o codice
configManager.setSensorPin("dht", 17);
configManager.setOutputPin("relay_1", 32);
configManager.setDisplayPin("oled_sda", 4);

// Riavvio richiesto per applicare
ESP.restart();
```

### Validare Pin

```cpp
if (configManager.validatePin(pin)) {
    // Pin valido per output
}

if (configManager.validatePin(pin, true)) {
    // Pin valido anche per input-only (34-39)
}
```

## File di Configurazione

I pin sono salvati in `/config.json` su LittleFS:

```json
{
  "pins": {
    "dht": 16,
    "bme_sda": 21,
    "bme_scl": 22,
    ...
  }
}
```

## Troubleshooting

### Pin non funzionano dopo modifica

1. Verificare che ESP32 sia stato riavviato
2. Controllare log seriale per errori
3. Verificare che i pin non siano in conflitto

### Validazione fallisce

- GPIO 6-11 sono riservati per Flash (non modificabili)
- GPIO 34-39 solo per input (encoder, sensori digitali)
- Alcuni pin potrebbero essere usati da bootloader

### Reset configurazione

Se la configurazione è corrotta:

```cpp
// Via seriale
configManager.saveConfig(); // Salva defaults

// O cancellare file
LITTLEFS.remove("/config.json");
ESP.restart();
```

### Conflitti I2C/SPI

Attenzione a non usare gli stessi pin per periferiche diverse:
- **SPI**: SCK=18, MISO=19, MOSI=23 (condivisi da TFT e Thermocouple)
- **I2C**: SDA=21, SCL=22 (condivisi da BME280 e OLED)

Questo è OK - le periferiche sullo stesso bus possono condividere i pin.

## Best Practices

1. **Non modificare senza necessità**: I default funzionano per la maggior parte degli usi
2. **Documentare modifiche**: Annotare perché hai cambiato un pin
3. **Testare subito**: Verificare funzionamento dopo ogni modifica
4. **Backup configurazione**: Salvare `config.json` prima di modifiche importanti
5. **Pin consecutivi**: Usare pin vicini per cablaggio più pulito

## Esempi Applicazioni

### Reflow Oven
```
Termocoppia MAX31855: CS=5 (vicino a SCK=18, MISO=19)
SSR relay: GPIO 25 (alto corrente disponibile)
Display TFT: Configurazione standard
```

### Fermentatore
```
DS18B20: GPIO 4 (lontano da fonti calore)
Relay heating: GPIO 27
Relay cooling: GPIO 14
Display OLED: SDA=21, SCL=22
```

### Multi-sensore
```
3x DS18B20 su stesso bus: GPIO 26
BME280: SDA=21, SCL=22
4x Relay: GPIO 27, 14, 12, 13
```

## Sicurezza

⚠️ **ATTENZIONE**:
- Non esporre l'interfaccia web a Internet senza autenticazione
- L'API non ha protezione password (aggiungere se necessario)
- Pin errati possono danneggiare hardware

## Riferimenti

- [ESP32 Pinout](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ESP32 Pin Restrictions](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html)
