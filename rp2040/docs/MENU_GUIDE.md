# Guida Sistema Menu - RP2040 PID Controller

Documentazione completa del sistema menu per la configurazione del controller PID.

## Indice
- [Controlli](#controlli)
- [Navigazione](#navigazione)
- [Menu Principali](#menu-principali)
- [Modifica Valori](#modifica-valori)
- [Esempi Utilizzo](#esempi-utilizzo)

---

## Controlli

### Encoder Rotativo

Il componente principale per la navigazione è l'encoder rotativo con pulsante integrato.

```
┌─────────────────┐
│  Encoder        │
│                 │
│  CLK  (GP6)     │ ← Segnale A
│  DT   (GP7)     │ ← Segnale B
│  SW   (GP8)     │ ← Pulsante (click)
│  GND            │
└─────────────────┘
```

**Azioni:**
- **Ruota CW (orario)**: Incrementa/Scorre giù
- **Ruota CCW (antiorario)**: Decrementa/Scorre su
- **Click breve**: Seleziona/Conferma
- **Tieni 1 secondo**: Entra/Esci menu

### Pulsanti Aggiuntivi

```
GP9  → Back Button  (torna indietro)
GP10 → Up Button    (opzionale - scorre su)
GP11 → Down Button  (opzionale - scorre giù)
```

**Nota**: I pulsanti UP/DOWN sono opzionali. L'encoder può gestire tutta la navigazione.

---

## Navigazione

### Schermata Principale

```
╔══════════════════════════╗
║    PID Controller        ║
╠══════════════════════════╣
║ Setpoint:  25.0°C        ║
║ Current:   24.5°C        ║
║ Output:    128 (50%)     ║
║ ████████████░░░░░░░░░░  ║
║ Mode: AUTO              ║
╚══════════════════════════╝
```

**Controlli Schermata Principale:**
- **Ruota Encoder**: Regola setpoint (±0.5°C per step)
- **Click Encoder**: Toggle AUTO ↔ MANUAL
- **Tieni Encoder 1s**: Entra in menu
- **Pulsante Back**: Entra in menu (alternativo)

### Menu Principale

```
╔══════════════════════════╗
║      Main Menu           ║
╠══════════════════════════╣
║ > PID Settings          ║
║   Sensor Select         ║
║   Calibration           ║
║   Relay Settings        ║
║   System Info           ║
║   Save/Load             ║
║   Exit Menu             ║
╚══════════════════════════╝
```

**Controlli Menu:**
- **Ruota Encoder**: Naviga tra opzioni
- **Click Encoder**: Entra nel sottomenu
- **Pulsante Back**: Torna indietro
- **Timeout 30s**: Ritorna automaticamente alla schermata principale

### Indicatori Scroll

Quando ci sono più opzioni della capacità del display:

```
╔══════════════════════════╗
║      Menu Name       ▲   ║  ← Freccia su: ci sono opzioni sopra
╠══════════════════════════╣
║   Option 3              ║
║ > Option 4              ║  ← Selezione corrente
║   Option 5              ║
║   Option 6              ║
║                      ▼   ║  ← Freccia giù: ci sono opzioni sotto
╚══════════════════════════╝
```

---

## Menu Principali

### 1. PID Settings

Configurazione controller PID.

```
╔══════════════════════════╗
║    PID Settings          ║
╠══════════════════════════╣
║ > Mode:      AUTO        ║
║   Setpoint:  25.0        ║
║   Kp:        2.00        ║
║   Ki:        5.00        ║
║   Kd:        1.00        ║
║   Reset PID              ║
╚══════════════════════════╝
```

**Opzioni:**

1. **Mode**: Toggle AUTO/MANUAL
   - AUTO: PID controlla output automaticamente
   - MANUAL: Output fisso (regolabile manualmente)

2. **Setpoint**: Valore target (0-200°C)
   - Step: 0.5°C
   - Click per modificare

3. **Kp**: Guadagno proporzionale (0-100)
   - Step: 0.1
   - Risposta rapida a errori

4. **Ki**: Guadagno integrale (0-100)
   - Step: 0.1
   - Elimina errore stazionario

5. **Kd**: Guadagno derivativo (0-100)
   - Step: 0.1
   - Riduce overshoot

6. **Reset PID**: Azzera integrale e stato interno

---

### 2. Sensor Select

Selezione sensore attivo.

```
╔══════════════════════════╗
║    Select Sensor         ║
║ Current: MAX31855        ║
╠══════════════════════════╣
║   DHT22                  ║
║   BME280                 ║
║ > MAX31855               ║
║   MAX6675                ║
║   DS18B20                ║
║   Back                   ║
╚══════════════════════════╝
```

**Sensori Disponibili:**
- DHT22: -40°C a +80°C (umidità inclusa)
- BME280: -40°C a +85°C (umidità + pressione)
- MAX31855: -200°C a +1350°C (termocoppia K)
- MAX6675: 0°C a +1024°C (termocoppia K)
- DS18B20: -55°C a +125°C (waterproof)

**Nota**: Solo sensori rilevati all'avvio sono selezionabili.

---

### 3. Calibration

Sistema di calibrazione sensore.

```
╔══════════════════════════╗
║    Calibration           ║
╠══════════════════════════╣
║ > Offset:    +1.50       ║
║   Scale:     1.000       ║
║   Enabled:   ON          ║
║   Auto-Cal:  Start       ║
╚══════════════════════════╝
```

**Opzioni:**

1. **Offset**: Correzione additiva (-50 a +50°C)
   - Formula: `Calibrated = Raw + Offset`
   - Step: 0.1°C

2. **Scale**: Correzione moltiplicativa (0.5 a 1.5)
   - Formula: `Calibrated = Raw × Scale`
   - Step: 0.01
   - Default: 1.000 (nessuna correzione)

3. **Enabled**: Attiva/disattiva calibrazione
   - ON: Applica correzioni
   - OFF: Usa valore grezzo

4. **Auto-Cal**: Calibrazione automatica
   - Click per inserire temperatura di riferimento
   - Sistema raccoglie 100 campioni (100 secondi)
   - Calcola offset automaticamente

**Procedura Auto-Calibrazione:**
```
1. Posiziona sensore in ambiente controllato
2. Attendi stabilizzazione (5+ minuti)
3. Menu → Calibration → Auto-Cal
4. Inserisci temp di riferimento (es. 25.0°C)
5. Attendi completamento (100s)
6. Sistema applica automaticamente offset calcolato
```

---

### 4. Relay Settings

Configurazione controllo relé.

```
╔══════════════════════════╗
║    Relay Settings        ║
╠══════════════════════════╣
║ > Mode:      DUAL        ║
║   Hysteresis: 0.5        ║
║   Time Prop:  OFF        ║
║   Manual Ctrl            ║
╚══════════════════════════╝
```

**Modalità Relé:**

1. **OFF**: Tutti relé disattivati
2. **HEAT**: Solo riscaldamento (relé 1)
   - ON quando temp < setpoint - hysteresis
   - OFF quando temp > setpoint + hysteresis
3. **COOL**: Solo raffreddamento (relé 2)
   - ON quando temp > setpoint + hysteresis
   - OFF quando temp < setpoint - hysteresis
4. **DUAL**: Riscaldamento + raffreddamento
   - Relé 1 per riscaldare
   - Relé 2 per raffreddare
   - Deadband = 2 × hysteresis

**Hysteresis**: Banda morta attorno al setpoint (0.1-10°C)
- Valore basso: Temperatura più stabile, più cicli relé
- Valore alto: Meno cicli relé, temperatura meno precisa

**Time Proportioning**: PWM lento per relé
- Cicli on/off proporzionali a output PID
- Migliore controllo rispetto a semplice ON/OFF

---

### 5. System Info

Informazioni sistema.

```
╔══════════════════════════╗
║     System Info          ║
╠══════════════════════════╣
║ Device:   RP2040-PID     ║
║ Version:  v1.0.0         ║
║ Temp:     24.5°C         ║
║ Uptime:   1234 ms        ║
║ Free RAM: 180 KB         ║
║ Sensor:   MAX31855       ║
╚══════════════════════════╝
```

**Informazioni Visualizzate:**
- Device: Nome dispositivo
- Version: Versione firmware
- Temp: Temperatura corrente
- Uptime: Millisecondi da avvio
- Free RAM: Memoria disponibile
- Sensor: Sensore attualmente attivo

**Utilità**: Diagnostica e verifica funzionamento.

---

### 6. Save/Load

Gestione configurazione.

```
╔══════════════════════════╗
║     Save/Load            ║
╠══════════════════════════╣
║ > Save Config            ║
║   Load Config            ║
║   Reset to Default       ║
║   Reboot System          ║
╚══════════════════════════╝
```

**Opzioni:**

1. **Save Config**: Salva configurazione corrente in EEPROM
   - Include: PID params, calibrazione, relé, sensore attivo
   - Conferma con messaggio

2. **Load Config**: Ricarica da EEPROM
   - Ripristina ultima configurazione salvata
   - Utile dopo modifiche sperimentali

3. **Reset to Default**: Ripristina valori di fabbrica
   - Cancella calibrazione
   - Reset parametri PID a default
   - Richiede conferma (doppio click)

4. **Reboot System**: Riavvia RP2040
   - Utile dopo modifiche importanti
   - Conferma richiesta

**Nota**: Ogni modifica nei menu viene salvata automaticamente. "Save Config" è un backup manuale aggiuntivo.

---

## Modifica Valori

### Processo Edit

Quando si modifica un valore numerico:

**Passo 1 - Seleziona**:
```
╔══════════════════════════╗
║ > Kp:        2.00        ║  ← Selezionato
╚══════════════════════════╝
```

**Passo 2 - Entra in Edit** (click encoder):
```
╔══════════════════════════╗
║   Kp:        2.00        ║  ← Lampeggia (edit mode)
╚══════════════════════════╝
```

**Passo 3 - Modifica** (ruota encoder):
```
╔══════════════════════════╗
║   Kp:        2.50        ║  ← Valore cambia
╚══════════════════════════╝
```

**Passo 4 - Conferma** (click encoder):
```
╔══════════════════════════╗
║   Kp:        2.50        ║  ← Salvato!
╚══════════════════════════╝
```

**Passo 4 Alt - Annulla** (pulsante back):
```
╔══════════════════════════╗
║   Kp:        2.00        ║  ← Ripristinato
╚══════════════════════════╝
```

### Range e Step

Ogni valore ha limiti e incrementi specifici:

| Parametro | Min | Max | Step | Unità |
|-----------|-----|-----|------|-------|
| Setpoint | 0 | 200 | 0.5 | °C |
| Kp | 0 | 100 | 0.1 | - |
| Ki | 0 | 100 | 0.1 | - |
| Kd | 0 | 100 | 0.1 | - |
| Hysteresis | 0.1 | 10.0 | 0.1 | °C |
| Cal Offset | -50 | +50 | 0.1 | °C |
| Cal Scale | 0.5 | 1.5 | 0.01 | - |
| Cal Reference | -50 | 200 | 0.5 | °C |

---

## Esempi Utilizzo

### Esempio 1: Configurazione Incubatrice

**Obiettivo**: Temperatura 37.5°C ± 0.2°C

```
1. Menu → PID Settings
   - Setpoint: 37.5
   - Kp: 10.0 (risposta rapida)
   - Ki: 2.0
   - Kd: 3.0
   - Mode: AUTO

2. Menu → Sensor Select
   - Seleziona: DHT22 (o DS18B20)

3. Menu → Relay Settings
   - Mode: HEAT
   - Hysteresis: 0.2

4. Menu → Save/Load → Save Config
```

### Esempio 2: Forno Reflow

**Obiettivo**: Controllo termocoppia fino a 250°C

```
1. Menu → Sensor Select
   - Seleziona: MAX31855

2. Menu → Calibration → Auto-Cal
   - Inserisci: 25.0 (temp ambiente)
   - Attendi 100 campioni

3. Menu → PID Settings
   - Setpoint: 150.0 (preheat)
   - Kp: 15.0 (aggressivo)
   - Ki: 3.0
   - Kd: 8.0
   - Mode: AUTO

4. Menu → Relay Settings
   - Mode: HEAT
   - Hysteresis: 2.0
   - Time Prop: ON

5. Menu → Save/Load → Save Config
```

### Esempio 3: Fermentatore Dual-Mode

**Obiettivo**: Mantieni 20°C con heating + cooling

```
1. Menu → Sensor Select
   - Seleziona: DS18B20

2. Menu → PID Settings
   - Setpoint: 20.0
   - Kp: 5.0 (moderato)
   - Ki: 1.0
   - Kd: 2.0
   - Mode: AUTO

3. Menu → Relay Settings
   - Mode: DUAL
   - Hysteresis: 0.5

4. Menu → Calibration
   - Auto-Cal con riferimento 20.0°C

5. Menu → Save/Load → Save Config
```

---

## Tips & Tricks

### Navigazione Veloce

- **Doppio Click**: Su alcuni menu, doppio click = conferma rapida
- **Rotazione Veloce**: Ruotare encoder velocemente = incrementi più grandi
- **Tieni Back**: Tenere premuto Back = esci da qualsiasi sottomenu

### Timeout Menu

- Il menu torna alla schermata principale dopo 30s di inattività
- Qualsiasi interazione resetta il timer
- Utile per evitare modifiche accidentali

### Indicatore Editing

Il valore in edit **lampeggia** ogni 500ms:
- ON: Valore visibile
- OFF: Valore nascosto (cursore)

Questo indica chiaramente quando sei in modalità edit.

### Salvataggio Automatico

Ogni modifica confermata viene salvata immediatamente in EEPROM.
Non è necessario "Save Config" dopo ogni modifica.

"Save Config" è utile per:
- Backup completo prima di esperimenti
- Conferma visiva del salvataggio
- Debug (verifica che EEPROM funzioni)

---

## Troubleshooting Menu

### Menu non risponde

1. Verificare encoder connessione:
   - GP6 (CLK)
   - GP7 (DT)
   - GP8 (SW)
   - GND

2. Controllare ENCODER_SENSITIVITY in config.h
   - Default: 4
   - Aumentare se troppo sensibile
   - Ridurre se poco responsivo

### Encoder scorre troppo veloce

Modifica `config.h`:
```cpp
#define ENCODER_SENSITIVITY 8  // Default: 4
```

### Menu timeout troppo breve/lungo

Modifica `config.h`:
```cpp
#define MENU_TIMEOUT 60000  // Default: 30000 (30s)
```

### Pulsanti non funzionano

1. Verificare pull-up attivi (default enabled)
2. Controllare DEBOUNCE_DELAY:
```cpp
#define DEBOUNCE_DELAY 50  // Default 50ms
```

### Valore non si salva

- EEPROM.commit() viene chiamato dopo ogni modifica
- Verificare con: Menu → System Info
- Se persiste: Menu → Save/Load → Reset to Default

---

## Personalizzazione

### Aggiungere Voci Menu

In `MenuSystem.cpp`, modificare `renderMainMenu()`:

```cpp
display->showMenu("Main Menu", selectedIndex, {
    "PID Settings",
    "Sensor Select",
    // ... opzioni esistenti ...
    "La Mia Opzione",  // ← Nuova voce
    "Exit Menu"
});
```

Gestire selezione in `selectItem()`.

### Modificare Layout Display

In `DisplayManager.cpp`, personalizzare rendering:

```cpp
void DisplayManager::showMainScreen(...) {
    // Personalizza layout, font, colori
}
```

### Nuovi Parametri Editabili

1. Aggiungi in `EditMode` enum (MenuSystem.h)
2. Implementa gestione in `confirmEdit()`
3. Aggiungi opzione menu in relativo sottomenu

---

## Riferimenti

- [Encoder Rotativo](https://lastminuteengineers.com/rotary-encoder-arduino-tutorial/)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Raspberry Pi Pico Pinout](https://pinout.xyz/pinout/raspberry_pi_pico)
