# Esempi di Utilizzo

Questa guida contiene esempi pratici per diverse applicazioni del controller PID ESP32.

## Indice
- [Controllo Temperatura](#controllo-temperatura)
- [Controllo Posizione](#controllo-posizione)
- [Controllo Umidità](#controllo-umidità)
- [Tuning PID](#tuning-pid)

---

## Controllo Temperatura

### Scenario: Incubatrice / Camera Termica

**Hardware necessario**:
- ESP32
- DHT22 o BME280 (sensore temperatura)
- SSR (Solid State Relay) o MOSFET
- Elemento riscaldante (resistenza, lampada, etc.)
- Display (TFT o OLED)

**Configurazione**:

```cpp
// In config.h
#define DEFAULT_KP     10.0  // Risposta rapida
#define DEFAULT_KI     0.5   // Elimina errore stazionario
#define DEFAULT_KD     5.0   // Riduce overshoot
#define DEFAULT_SETPOINT 37.5 // Target: 37.5°C
```

**Collegamenti**:
```
ESP32 GPIO 25 ──> SSR ──> Riscaldatore ──> 220V AC
DHT22 DATA ──> ESP32 GPIO 16
```

**Procedura**:
1. Configurare setpoint a 37.5°C via web
2. Attivare modalità AUTO
3. Monitorare temperatura sul display
4. Output PWM controlla SSR (0-100%)

**Risultato atteso**:
- Raggiunge setpoint in 5-10 minuti
- Mantiene temperatura ±0.5°C
- Senza oscillazioni eccessive

---

## Controllo Posizione

### Scenario: Posizionamento Servo/Motore

**Hardware necessario**:
- ESP32
- Encoder rotativo (feedback posizione)
- Motor driver (L298N, DRV8825)
- Motore DC o stepper

**Configurazione**:

```cpp
// In config.h
#define DEFAULT_KP     3.0   // Risposta proporzionale
#define DEFAULT_KI     0.1   // Minimo integrale
#define DEFAULT_KD     2.0   // Smorzamento
#define DEFAULT_SETPOINT 0.0 // Posizione target
```

**Modifiche al codice**:

```cpp
// In main.cpp, modificare updatePID():
void updatePID() {
    // Usa posizione encoder come input
    currentInput = sensors.getEncoderPosition();

    if (pid.isAuto()) {
        currentOutput = pid.compute(currentInput);

        // Converti output in direzione e velocità
        int direction = (currentOutput > 127) ? HIGH : LOW;
        int speed = abs(currentOutput - 127) * 2;

        digitalWrite(MOTOR_DIR_PIN, direction);
        ledcWrite(0, speed);
    }
}
```

**Risultato atteso**:
- Raggiunge posizione target precisamente
- Nessuna oscillazione attorno al target
- Risposta rapida a cambiamenti setpoint

---

## Controllo Umidità

### Scenario: Serra / Grow Box

**Hardware necessario**:
- ESP32
- BME280 (sensore umidità)
- Relè
- Umidificatore o deumidificatore

**Configurazione**:

```cpp
// In config.h
#define DEFAULT_KP     5.0   // Moderato
#define DEFAULT_KI     1.0   // Elimina drift
#define DEFAULT_KD     0.5   // Leggero smorzamento
#define DEFAULT_SETPOINT 60.0 // Target: 60% RH
```

**Modifiche al codice**:

```cpp
// In main.cpp
void updatePID() {
    // Usa umidità come input
    currentInput = sensors.getHumidity();

    if (pid.isAuto()) {
        currentOutput = pid.compute(currentInput);

        // ON/OFF control (bang-bang con PWM)
        if (currentOutput > 128) {
            // Accendi umidificatore
            digitalWrite(HUMIDIFIER_PIN, HIGH);
        } else {
            // Spegni umidificatore
            digitalWrite(HUMIDIFIER_PIN, LOW);
        }
    }
}
```

**Miglioramento**: Usare PWM per controllo più fine (cicli on/off rapidi).

---

## Tuning PID

### Metodo Ziegler-Nichols

**Step 1: Trova Ku (Guadagno Critico)**

1. Imposta Ki=0, Kd=0
2. Aumenta Kp gradualmente
3. Osserva oscillazioni
4. Quando oscillazioni costanti: Ku = Kp attuale

**Step 2: Misura Tu (Periodo Oscillazione)**

- Misura tempo tra due picchi
- Tu = periodo oscillazione in secondi

**Step 3: Calcola parametri**

Per controllo PID classico:
```
Kp = 0.6 * Ku
Ki = 1.2 * Ku / Tu
Kd = 0.075 * Ku * Tu
```

**Esempio pratico**:
```
Ku = 20.0 (oscillazioni a Kp=20)
Tu = 4.0 secondi

Kp = 0.6 * 20 = 12.0
Ki = 1.2 * 20 / 4 = 6.0
Kd = 0.075 * 20 * 4 = 6.0
```

### Metodo Trial-and-Error

**Procedura**:

1. **Solo P (Kp)**:
   ```
   Ki = 0, Kd = 0, Kp = 1
   Aumentare Kp finché sistema risponde rapidamente
   Ridurre se oscilla troppo
   ```

2. **Aggiungi I (Ki)**:
   ```
   Kd = 0, Kp = [dal passo 1]
   Aumentare Ki per eliminare errore residuo
   Ridurre se sistema diventa instabile
   ```

3. **Aggiungi D (Kd)**:
   ```
   Kp = [passo 1], Ki = [passo 2]
   Aumentare Kd per ridurre overshoot
   Ridurre se sistema diventa "nervoso"
   ```

### Tuning via Web Interface

```javascript
// Apri console browser su http://[ESP32_IP]

// Test Kp
ws.send(JSON.stringify({cmd: 'setPID', kp: 5.0, ki: 0, kd: 0}));

// Osserva risposta, poi test Ki
ws.send(JSON.stringify({cmd: 'setPID', kp: 5.0, ki: 1.0, kd: 0}));

// Infine Kd
ws.send(JSON.stringify({cmd: 'setPID', kp: 5.0, ki: 1.0, kd: 2.0}));
```

---

## Applicazioni Avanzate

### 1. Reflow Oven (Forno per Saldatura)

**Profilo temperatura**:
```
Preheat: 150°C per 60s
Soak:    180°C per 90s
Reflow:  235°C per 30s
Cooling: Rampa discesa
```

**Implementazione**:
- Array di setpoint con timing
- Cambio setpoint automatico
- Allarme se fuori tolleranza

### 2. Fermentatore Birra

**Controllo dual-mode**:
- Riscaldamento (resistenza)
- Raffreddamento (Peltier)
- Setpoint variabile (profilo fermentazione)

### 3. Stampante 3D (Hotend)

**Caratteristiche**:
- Risposta molto rapida (Kp alto)
- Controllo stretto (±1°C)
- Safety: spegni se >250°C

---

## Monitoraggio e Debug

### Serial Monitor

Aggiungere debug in `main.cpp`:

```cpp
void loop() {
    // ... codice esistente ...

    // Debug PID ogni secondo
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 1000) {
        Serial.printf("SP:%.1f IN:%.1f OUT:%.0f ERR:%.1f\n",
            pid.getSetpoint(),
            currentInput,
            currentOutput,
            pid.getSetpoint() - currentInput
        );
        lastDebug = millis();
    }
}
```

### Web Dashboard

Già implementato! Aprire browser:
```
http://[ESP32_IP]
```

- Grafico in tempo reale (da implementare con Chart.js)
- Storico valori
- Export dati CSV

### Log su SD Card

Aggiungere SD card logger:

```cpp
#include <SD.h>

File logFile = SD.open("/pid_log.csv", FILE_WRITE);
if (logFile) {
    logFile.printf("%lu,%.2f,%.2f,%.2f\n",
        millis(), currentInput, currentOutput, pid.getSetpoint());
    logFile.close();
}
```

---

## Tips & Tricks

### 1. Anti-Windup

Già implementato! L'integrale è limitato per evitare saturazione.

### 2. Bumpless Transfer

Quando si passa da MANUAL a AUTO, il sistema resetta l'integrale per evitare salti.

### 3. Setpoint Ramping

Per evitare shock termici:

```cpp
double targetSetpoint = 100.0;
double currentSetpoint = pid.getSetpoint();
double rampRate = 1.0; // °C per secondo

if (currentSetpoint < targetSetpoint) {
    currentSetpoint += rampRate * (PID_UPDATE_INTERVAL / 1000.0);
    if (currentSetpoint > targetSetpoint) currentSetpoint = targetSetpoint;
    pid.setSetpoint(currentSetpoint);
}
```

### 4. Deadband

Ignora piccole variazioni:

```cpp
double error = pid.getSetpoint() - currentInput;
if (abs(error) < 0.5) {
    // Non fare nulla, sistema stabile
    return;
}
```

---

## Progetti Suggeriti

1. **Sous-vide Cooker**: Controllo temperatura acqua ±0.1°C
2. **Coffee Roaster**: Profilo temperatura complesso
3. **Greenhouse Controller**: Multi-sensore (temp, umidità, luce)
4. **Aquarium Controller**: Temperatura + pH
5. **Drone Stabilizer**: Controllo angolo (con IMU)

---

## Risorse

- [PID Without a PhD](https://www.wescottdesign.com/articles/pid/pidWithoutAPhD.pdf)
- [Arduino PID Library](https://github.com/br3ttb/Arduino-PID-Library)
- [PID Tuning Guide](https://www.crossco.com/blog/basics-tuning-pid-loops/)

---

## Contributi

Hai creato un'applicazione interessante? Condividila!

Apri una issue su GitHub con:
- Descrizione applicazione
- Schema hardware
- Parametri PID usati
- Foto/Video del risultato
