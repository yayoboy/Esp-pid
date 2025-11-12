# Navigazione Display con 4 Pulsanti (OLED)

Il display OLED (SSD1315) può essere navigato usando 4 pulsanti fisici per visualizzare diverse pagine di informazioni e controllare il sistema PID.

## Configurazione Hardware

### Pin dei Pulsanti (Default)

I 4 pulsanti sono collegati ai seguenti pin GPIO dell'ESP32:

| Pulsante | GPIO | Funzione |
|----------|------|----------|
| **UP** | GPIO 13 | Pagina precedente |
| **DOWN** | GPIO 12 | Pagina successiva |
| **SELECT** | GPIO 14 | Seleziona/Conferma |
| **BACK** | GPIO 27 | Torna alla pagina principale |

### Schema di Collegamento

```
ESP32          Pulsante
-----          --------
GPIO 13   -->  UP (con pull-up interno)
GPIO 12   -->  DOWN (con pull-up interno)
GPIO 14   -->  SELECT (con pull-up interno)
GPIO 27   -->  BACK (con pull-up interno)
GND       -->  Comune pulsanti
```

**Nota**: I pulsanti devono collegare il GPIO a GND quando premuti. I pull-up interni dell'ESP32 sono già attivati automaticamente.

### Personalizzazione Pin

Per modificare i pin dei pulsanti, edita `include/config.h`:

```cpp
#ifdef USE_OLED_DISPLAY
    // 4 Navigation Buttons for OLED
    #define BTN_UP      13   // Cambia qui
    #define BTN_DOWN    12   // Cambia qui
    #define BTN_SELECT  14   // Cambia qui
    #define BTN_BACK    27   // Cambia qui
#endif
```

## Funzionalità dei Pulsanti

### 🔼 Pulsante UP

**Pressione breve**: Naviga alla pagina precedente

Sequenza pagine (all'indietro):
```
NETWORK → SENSORS → TUNING → SETTINGS → MAIN → (torna a NETWORK)
```

### 🔽 Pulsante DOWN

**Pressione breve**: Naviga alla pagina successiva

Sequenza pagine (in avanti):
```
MAIN → SETTINGS → TUNING → SENSORS → NETWORK → (torna a MAIN)
```

### ✅ Pulsante SELECT

**Pressione breve** (su pagina MAIN):
- Cambia modalità PID: **AUTO** ⟷ **MANUAL**
- La modifica viene salvata automaticamente

**Pressione lunga** (1 secondo):
- Riservato per future funzionalità (menu configurazione)
- Attualmente mostra solo un messaggio di debug

### ◀️ Pulsante BACK

**Pressione breve**: Torna direttamente alla pagina MAIN da qualsiasi schermata

Utile per tornare rapidamente alla visualizzazione principale.

## Pagine Disponibili

### 1. PAGE_MAIN - Pagina Principale

**Visualizzazione**:
- Valore corrente (Input)
- Setpoint desiderato
- Output PID (0-255)
- Modalità: AUTO o MANUAL

**Azioni disponibili**:
- SELECT: Cambia modalità AUTO/MANUAL

### 2. PAGE_SETTINGS - Impostazioni

**Visualizzazione**:
- Configurazioni generali
- Stato sistema

### 3. PAGE_TUNING - Parametri PID

**Visualizzazione**:
- Kp (Proportional)
- Ki (Integral)
- Kd (Derivative)

### 4. PAGE_SENSORS - Sensori

**Visualizzazione**:
- Temperatura (°C)
- Umidità (%) - se disponibile
- Posizione encoder - se disponibile
- Tipo sensore attivo

### 5. PAGE_NETWORK - Rete

**Visualizzazione**:
- Indirizzo IP
- Stato connessione WiFi
- Modalità (STA o AP)

## Comportamento Pulsanti

### Debouncing Automatico

Tutti i pulsanti hanno **debouncing hardware** di 50ms per evitare letture multiple accidentali.

### Rilevamento Edge

I pulsanti usano **edge detection**: la pressione viene rilevata solo quando il pulsante passa da rilasciato a premuto. Questo previene azioni ripetute se il pulsante viene tenuto premuto.

### Long Press

La **pressione lunga** viene rilevata dopo 1000ms (1 secondo) di pressione continua.

## Debug e Monitoraggio

### Output Seriale

Quando i pulsanti vengono premuti, il sistema stampa messaggi di debug sulla seriale:

```
[Buttons] Initialized with pins:
  UP=13, DOWN=12, SELECT=14, BACK=27

[Buttons] Page changed to: 1
[Buttons] Page changed to: 0
[Buttons] PID mode changed to: AUTO
[Buttons] Returned to main page
[Buttons] SELECT long press - configuration mode (not implemented)
```

### Verifica Funzionamento

1. Apri il monitor seriale (115200 baud)
2. All'avvio dovresti vedere: `[Buttons] Initialized with pins: ...`
3. Premi i pulsanti e verifica i messaggi di debug
4. Controlla che il display cambi pagina

## Risoluzione Problemi

### Pulsanti Non Rispondono

**Sintomi**: Nessuna reazione quando si premono i pulsanti

**Soluzioni**:
1. Verifica i collegamenti hardware
2. Controlla che `USE_OLED_DISPLAY` sia definito in `config.h`
3. Verifica GPIO corretti nel monitor seriale all'avvio
4. Testa i pulsanti con un multimetro (devono cortocircuitare GPIO a GND)

### Pulsanti Rispondono in Modo Errato

**Sintomi**: Pulsante UP fa azione DOWN, o simili

**Soluzioni**:
1. Verifica i pin nel monitor seriale
2. Controlla collegamenti hardware (GPIO corretto?)
3. Verifica definizioni in `config.h`

### Pressioni Multiple

**Sintomi**: Una pressione viene rilevata 2-3 volte

**Soluzioni**:
1. Il debouncing è già attivo (50ms)
2. Prova ad aumentare `BUTTON_DEBOUNCE_MS` in `config.h`:
   ```cpp
   #define BUTTON_DEBOUNCE_MS 100  // Aumenta a 100ms
   ```
3. Verifica qualità pulsanti (potrebbero avere bounce eccessivo)

### Display Non Aggiorna

**Sintomi**: Pulsanti rilevati ma display non cambia

**Soluzioni**:
1. Verifica che display sia inizializzato correttamente
2. Controlla messaggi debug su seriale
3. Il display si aggiorna ogni 500ms - attendi un momento

## Personalizzazione Avanzata

### Modificare Debounce Time

In `config.h`:
```cpp
#define BUTTON_DEBOUNCE_MS 50  // Default 50ms, aumenta se problemi
```

### Modificare Long Press Time

In `config.h`:
```cpp
#define BUTTON_LONG_PRESS_MS 1000  // Default 1 secondo
```

### Aggiungere Nuove Azioni

Per aggiungere comportamenti custom ai pulsanti, modifica la funzione `handleButtons()` in `src/main.cpp`:

```cpp
#ifdef USE_OLED_DISPLAY
void handleButtons() {
    // Aggiungi qui le tue azioni custom
    if (buttons.isUpPressed()) {
        // Azione custom per UP
    }

    if (buttons.isUpLongPressed()) {
        // Azione custom per UP long press
    }

    // ... altro codice
}
#endif
```

### API ButtonHandler

Il ButtonHandler fornisce questi metodi:

```cpp
// Edge detection (true una sola volta per pressione)
bool isUpPressed();
bool isDownPressed();
bool isSelectPressed();
bool isBackPressed();

// Stato corrente (true finché premuto)
bool isUpHeld();
bool isDownHeld();
bool isSelectHeld();
bool isBackHeld();

// Long press detection
bool isUpLongPressed();
bool isDownLongPressed();
bool isSelectLongPressed();
bool isBackLongPressed();

// Utility
bool anyButtonPressed();  // true se qualsiasi pulsante premuto
```

## Funzionalità Future

### Modifica Valori (Prossima Implementazione)

**Obiettivo**: Usare UP/DOWN per modificare valori (setpoint, Kp, Ki, Kd)

**Comportamento previsto**:
1. SELECT per entrare in modalità edit
2. UP/DOWN per incrementare/decrementare valore
3. SELECT per confermare o BACK per annullare

### Menu Configurazione

**Obiettivo**: Menu completo per configurare tutti i parametri

**Comportamento previsto**:
1. SELECT long press per entrare nel menu
2. UP/DOWN per navigare opzioni
3. SELECT per modificare
4. BACK per uscire

### Salvataggio Rapido

**Obiettivo**: Salvare configurazione corrente

**Comportamento previsto**:
- BACK long press per salvare configurazione

## Esempi Pratici

### Esempio 1: Navigare tra le Pagine

```
1. Sistema si avvia su PAGE_MAIN
2. Premi DOWN → PAGE_SETTINGS
3. Premi DOWN → PAGE_TUNING
4. Premi DOWN → PAGE_SENSORS
5. Premi BACK → PAGE_MAIN (shortcut)
```

### Esempio 2: Cambiare Modalità PID

```
1. Assicurati di essere su PAGE_MAIN (premi BACK)
2. Premi SELECT → Modalità cambia AUTO ⟷ MANUAL
3. Display mostra nuova modalità
4. Configurazione salvata automaticamente
```

### Esempio 3: Visualizzare Info Rete

```
1. Da PAGE_MAIN, premi DOWN 4 volte
2. Oppure: premi UP 1 volta (wrap-around)
3. Visualizzi PAGE_NETWORK con IP
```

## Integrazione con Web Interface

I pulsanti fisici e l'interfaccia web lavorano in parallelo:

- **Modifiche via pulsanti**: Visibili immediatamente sul web
- **Modifiche via web**: Visibili sul display dopo max 500ms
- **Configurazioni salvate**: Condivise tra pulsanti e web

## Compatibilità

### Display TFT Touch

I pulsanti fisici sono disponibili **SOLO** con display OLED. Il display TFT touch usa l'input touch screen, non i pulsanti.

### RP2040 Version

La versione RP2040 ha un sistema di navigazione separato con encoder rotativo. Consulta `rp2040/docs/MENU_GUIDE.md` per dettagli.

## Riferimenti

- **Codice sorgente**: `src/ButtonHandler.cpp`
- **Header**: `include/ButtonHandler.h`
- **Configurazione**: `include/config.h`
- **Logica navigazione**: `src/main.cpp` → `handleButtons()`

## Changelog

**v1.0** (2024-01-12):
- Navigazione base tra pagine con UP/DOWN
- Toggle modalità PID con SELECT
- Return to main con BACK
- Debouncing e long press detection
- Debug output seriale

---

**Prossimi Sviluppi**:
- [ ] Modifica valori con UP/DOWN in modalità edit
- [ ] Menu configurazione completo
- [ ] Calibrazione sensori via pulsanti
- [ ] Configurazione PIN via pulsanti
- [ ] Salvataggio rapido configurazione

---

**Ultima modifica**: 2024-01-12
**Versione**: 1.0.0
