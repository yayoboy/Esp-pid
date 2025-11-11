# Schema Collegamenti Dettagliato

## Indice
- [Display TFT 240x320](#display-tft-240x320)
- [Display OLED 0.96"](#display-oled-096)
- [Sensori](#sensori)
- [Output PWM](#output-pwm)
- [Alimentazione](#alimentazione)

## Display TFT 240x320

### ILI9341 / ST7789

Il display TFT utilizza l'interfaccia SPI per la comunicazione.

```
┌─────────────────┐
│     ESP32       │
├─────────────────┤         ┌──────────────────┐
│            3.3V │────────>│ VCC              │
│             GND │────────>│ GND              │
│      GPIO 15    │────────>│ CS    (Chip Sel) │
│      GPIO 2     │────────>│ DC    (Data/Cmd) │
│      GPIO 4     │────────>│ RST   (Reset)    │
│      GPIO 23    │────────>│ MOSI  (SDA)      │
│      GPIO 18    │────────>│ SCK   (CLK)      │
│      GPIO 19    │<────────│ MISO             │
│      GPIO 5     │────────>│ T_CS  (Touch CS) │
│      GPIO 17    │<────────│ T_IRQ (Touch)    │
└─────────────────┘         │  TFT Display     │
                            └──────────────────┘
```

### Note:
- Alcuni display potrebbero non avere MISO, in questo caso non collegare
- Il touch è opzionale ma consigliato per l'interfaccia
- **IMPORTANTE**: Alimentare a 3.3V, non 5V!

### Modifica Driver (se necessario)

Se il display usa ST7789 invece di ILI9341, modificare in `config.h`:

```cpp
#define TFT_DRIVER ST7789
// #define TFT_DRIVER ILI9341
```

---

## Display OLED 0.96"

### SSD1315 / SSD1306

Il display OLED utilizza l'interfaccia I2C.

```
┌─────────────────┐
│     ESP32       │
├─────────────────┤         ┌──────────────────┐
│            3.3V │────────>│ VCC              │
│             GND │────────>│ GND              │
│      GPIO 21    │<───────>│ SDA              │
│      GPIO 22    │────────>│ SCL              │
└─────────────────┘         │  OLED Display    │
                            └──────────────────┘
```

### Tastiera 4x4 (Opzionale)

```
┌─────────────────┐
│     ESP32       │         Righe:
├─────────────────┤         ┌────────────┐
│      GPIO 13    │────────>│ R1         │
│      GPIO 12    │────────>│ R2         │
│      GPIO 14    │────────>│ R3         │
│      GPIO 27    │────────>│ R4         │
│                 │         └────────────┘
│      GPIO 26    │<────────┐
│      GPIO 25    │<────────│ Colonne
│      GPIO 33    │<────────│ C1 C2 C3 C4
│      GPIO 32    │<────────┘
└─────────────────┘
```

---

## Sensori

### DHT22 (Temperatura e Umidità)

```
┌─────────────────┐
│     ESP32       │
├─────────────────┤         ┌──────────────────┐
│            3.3V │────────>│ VCC (+)          │
│             GND │────────>│ GND (-)          │
│      GPIO 16    │<───────>│ DATA             │
└─────────────────┘         │     DHT22        │
                            └──────────────────┘
```

**Nota**: Aggiungere resistenza pull-up da 10kΩ tra DATA e VCC se necessario.

### BME280 (Temperatura, Umidità, Pressione)

```
┌─────────────────┐
│     ESP32       │
├─────────────────┤         ┌──────────────────┐
│            3.3V │────────>│ VCC              │
│             GND │────────>│ GND              │
│      GPIO 21    │<───────>│ SDA              │
│      GPIO 22    │────────>│ SCL              │
└─────────────────┘         │     BME280       │
                            └──────────────────┘
```

**Indirizzo I2C**: 0x76 o 0x77 (il software prova entrambi)

### Encoder Rotativo (Posizione)

```
┌─────────────────┐
│     ESP32       │
├─────────────────┤         ┌──────────────────┐
│             GND │────────>│ GND              │
│      GPIO 34    │<────────│ CLK (A)          │
│      GPIO 35    │<────────│ DT  (B)          │
│                 │         │ SW (bottone)     │ (opzionale)
└─────────────────┘         │  Rotary Encoder  │
                            └──────────────────┘
```

**Note**:
- GPIO 34 e 35 sono input-only, perfetti per encoder
- Il bottone centrale (SW) è opzionale e non ancora implementato

---

## Output PWM

### Controllo Attuatore

```
┌─────────────────┐
│     ESP32       │
├─────────────────┤         ┌──────────────────┐
│      GPIO 25    │────────>│ IN               │
│             GND │────────>│ GND              │
└─────────────────┘         │  Driver          │
                            │  (MOSFET/SSR)    │
                            └──────────────────┘
```

### Esempi di utilizzo:

**1. Riscaldatore (con SSR - Solid State Relay)**
```
GPIO 25 ──> SSR ──> Riscaldatore ──> AC/DC Power
```

**2. Motore DC (con driver MOSFET)**
```
GPIO 25 ──> MOSFET Driver ──> Motor ──> Power Supply
```

**3. Ventola PWM**
```
GPIO 25 ──> Fan PWM Control
```

**IMPORTANTE**:
- ESP32 fornisce max 40mA per pin
- Usare SEMPRE un driver (MOSFET, SSR, relay) per carichi superiori
- Non collegare carichi ad alta potenza direttamente all'ESP32!

---

## Alimentazione

### Opzione 1: USB (Sviluppo)
```
PC/Adapter ──[USB]──> ESP32
```
- Ideale per sviluppo e testing
- Limite corrente: ~500mA

### Opzione 2: Alimentatore Esterno
```
                    ┌─────────────┐
5V Power Supply ────┤ 5V     3.3V ├──> Sensori/Display (3.3V)
                    │             │
GND ────────────────┤ GND     GND ├──> GND comune
                    └─────────────┘
                         ESP32
```

**Raccomandazioni**:
- Alimentatore 5V / 2A minimo
- Condensatore 100µF tra VIN e GND per stabilità
- Per progetti definitivi, usare regolatore di tensione dedicato

### Calcolo Corrente

| Componente      | Corrente Tipica |
|-----------------|-----------------|
| ESP32           | ~240 mA         |
| TFT Display     | ~100 mA         |
| OLED Display    | ~30 mA          |
| DHT22           | ~2.5 mA         |
| BME280          | ~3.6 mA         |
| LED/Relay       | Varia           |

**Totale stimato**: 400-500 mA (senza attuatori esterni)

---

## Schema Completo (Esempio con TFT + BME280)

```
                              ┌──────────────────┐
                              │     ESP32        │
                              ├──────────────────┤
        ┌────────────────────>│ 3.3V             │
        │    ┌────────────────>│ GND              │
        │    │                 │                  │
        │    │  ┌─────────────>│ GPIO 15  (TFT CS)│
        │    │  │  ┌──────────>│ GPIO 2   (TFT DC)│
        │    │  │  │  ┌───────>│ GPIO 4   (TFT RST)│
        │    │  │  │  │  ┌────>│ GPIO 23  (MOSI)  │
        │    │  │  │  │  │  ┌─>│ GPIO 18  (SCK)   │
        │    │  │  │  │  │  │  │                  │
        │    │  │  │  │  │  │  │ GPIO 21  (SDA)   │<──┐
        │    │  │  │  │  │  │  │ GPIO 22  (SCL)   │<──│─┐
        │    │  │  │  │  │  │  │                  │   │ │
        │    │  │  │  │  │  │  │ GPIO 25  (PWM)   │───│─│──> Output
        │    │  │  │  │  │  │  └──────────────────┘   │ │
        │    │  │  │  │  │  │                         │ │
┌───────┴────┴──┴──┴──┴──┴──┴─────┐          ┌────────┴─┴────────┐
│ VCC GND CS DC RST MOSI SCK MISO │          │ VCC GND SDA SCL   │
│        TFT Display               │          │     BME280        │
└──────────────────────────────────┘          └───────────────────┘
```

---

## Checklist Pre-Test

Prima di alimentare il sistema, verificare:

- [ ] Tutti i GND sono collegati insieme
- [ ] Nessun cortocircuito tra VCC e GND
- [ ] Display alimentato a 3.3V (NON 5V!)
- [ ] Sensori I2C hanno GND comune
- [ ] Pin ESP32 corretti secondo `config.h`
- [ ] Alimentazione adeguata (minimo 2A)
- [ ] Cavi ben saldati/connessi

---

## Troubleshooting Collegamenti

### Display non si accende
- Verificare alimentazione 3.3V
- Controllare GND comune
- Testare continuità cavi

### Sensori non rilevati
- Verificare indirizzo I2C con scanner
- Controllare pull-up su SDA/SCL (4.7kΩ)
- Testare sensore singolarmente

### Letture errate
- Verificare GND comune
- Controllare interferenze (separare cavi di potenza)
- Aggiungere condensatori di disaccoppiamento

### ESP32 si riavvia
- Alimentazione insufficiente
- Aggiungere condensatore 100-470µF
- Verificare corrente disponibile

---

## Consigli Pratici

1. **Breadboard**: Usare per prototyping iniziale
2. **PCB**: Considerare per progetti definitivi
3. **Cavi corti**: Minimizzare lunghezza per ridurre rumore
4. **Separazione**: Tenere cavi di potenza lontani da segnali
5. **Condensatori**: Aggiungere 0.1µF vicino a ogni IC
6. **Testing**: Testare componenti uno alla volta

---

## Revisioni

- v1.0 - Schema iniziale
- v1.1 - Aggiunti dettagli alimentazione
- v1.2 - Sezione troubleshooting
