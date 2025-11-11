# Guida Controllo Relé

Documentazione completa per il controllo relé del controller PID ESP32.

## Indice
- [Introduzione](#introduzione)
- [Modalità Operative](#modalità-operative)
- [Collegamenti Hardware](#collegamenti-hardware)
- [Configurazione Software](#configurazione-software)
- [Time Proportioning](#time-proportioning)
- [Applicazioni Pratiche](#applicazioni-pratiche)

---

## Introduzione

Il controller supporta 4 relé indipendenti per il controllo di carichi elettrici. Il sistema può operare in diverse modalità per adattarsi a varie applicazioni.

### Vantaggi del Controllo a Relé

✅ **Pro**:
- Carichi ad alta potenza (fino a 10A+)
- Completa isolazione galvanica
- Compatibile con AC e DC
- Economico
- Robusto

❌ **Contro**:
- Tempo di risposta lento (ms)
- Usura meccanica
- Click udibile
- Cicli di vita limitati (100k-1M)

### Quando Usare Relé vs PWM

**Usare Relé per**:
- Carichi AC (220V)
- Alte correnti (> 2A)
- Controllo ON/OFF semplice
- Elementi riscaldanti
- Motori AC

**Usare PWM per**:
- Carichi DC a bassa tensione
- Controllo preciso continuo
- Alta frequenza necessaria
- Ventole DC
- LED

---

## Modalità Operative

### 1. RELAY_OFF

Tutti i relé disattivati. Modalità sicura.

```cpp
relayController.setMode(RELAY_OFF);
```

### 2. RELAY_HEATING_ONLY

Controllo solo riscaldamento con isteresi.

**Come funziona**:
```
Temperatura < Setpoint - Isteresi  → Relé ON
Temperatura > Setpoint + Isteresi  → Relé OFF
```

**Esempio**:
```
Setpoint: 25°C
Isteresi: 0.5°C

Temp < 24.5°C → Riscaldamento ON
Temp > 25.5°C → Riscaldamento OFF
```

**Configurazione**:
```cpp
relayController.setMode(RELAY_HEATING_ONLY);
relayController.setHysteresis(0.5); // ±0.5°C
```

**Applicazioni**:
- Incubatrici
- Forni
- Riscaldamento ambienti
- Termostatazione semplice

### 3. RELAY_COOLING_ONLY

Controllo solo raffreddamento con isteresi.

**Come funziona**:
```
Temperatura > Setpoint + Isteresi  → Relé ON (raffredda)
Temperatura < Setpoint - Isteresi  → Relé OFF
```

**Esempio**:
```
Setpoint: 4°C (frigo)
Isteresi: 0.5°C

Temp > 4.5°C → Raffreddamento ON
Temp < 3.5°C → Raffreddamento OFF
```

**Configurazione**:
```cpp
relayController.setMode(RELAY_COOLING_ONLY);
relayController.setHysteresis(0.5);
```

**Applicazioni**:
- Frigoriferi
- Raffreddamento fermentatori
- Climatizzazione estate
- Peltier coolers

### 4. RELAY_DUAL_MODE

Controllo sia riscaldamento che raffreddamento con deadband.

**Come funziona**:
```
Temperatura < Setpoint - Isteresi  → Riscaldamento ON
Temperatura > Setpoint + Isteresi  → Raffreddamento ON
Intermedio                         → Entrambi OFF
```

**Deadband**: Zona tra i due relé dove nessuno è attivo (evita cicli rapidi).

**Esempio**:
```
Setpoint: 20°C
Isteresi: 1.0°C

Temp < 19°C → Solo riscaldamento ON
19°C < Temp < 21°C → Entrambi OFF (deadband)
Temp > 21°C → Solo raffreddamento ON
```

**Configurazione**:
```cpp
relayController.setMode(RELAY_DUAL_MODE);
relayController.setHysteresis(1.0); // Deadband più largo
```

**Applicazioni**:
- Fermentatori con riscaldamento e raffreddamento
- Climatizzazione completa
- Camere ambientali
- Test climatici

---

## Collegamenti Hardware

### Schema Base

```
ESP32          Modulo Relé       Carico
GPIO 27 ──────> IN1         ──┐
GPIO 14 ──────> IN2         ──┤
GPIO 12 ──────> IN3         ──┤  Relé 1-4
GPIO 13 ──────> IN4         ──┘
5V      ──────> VCC
GND     ──────> GND


Relé 1 (Riscaldamento):
COM ──> Linea AC/DC
NO  ──> Elemento Riscaldante ──> Neutro/GND

Relé 2 (Raffreddamento):
COM ──> Linea AC/DC
NO  ──> Ventola/Compressore ──> Neutro/GND
```

### Modulo Relé 4 Canali

**Pin Out tipico**:
```
[VCC] [IN1] [IN2] [IN3] [IN4] [GND]
```

**Nota**: La maggior parte dei moduli sono **attivi LOW**!
```
HIGH (3.3V) → Relé OFF
LOW  (0V)   → Relé ON
```

Il software gestisce automaticamente la logica.

### Collegamento Elemento Riscaldante (AC)

⚠️ **ATTENZIONE: ALTA TENSIONE!**

```
       ┌──[Fusibile]──[Relé COM]──[Riscaldatore]──┐
       │                                           │
Fase ──┤                                           ├─ Neutro
       │              [Relé NO]                    │
       └───────────────────────────────────────────┘
```

**Sicurezza**:
- ✅ Usare fusibile adeguato
- ✅ Relé rated per AC (10A minimo)
- ✅ Cavi adeguati alla corrente
- ✅ Messa a terra
- ⚠️ NON toccare con alimentazione inserita!

### Collegamento Ventola/Compressore (DC)

```
Alimentatore 12V ──[+]──[Relé COM]──[Ventola +]
                   [-]──[GND]──────[Ventola -]
                        [Relé NO]
```

### SSR (Solid State Relay)

Per carichi ad alta potenza, preferire SSR:

```
ESP32 GPIO 27 ──[330Ω]──[SSR +] ──┐
GND ───────────────────[SSR -]    │
                                  │
                            Load Control
```

**Vantaggi SSR**:
- Silenzioso
- Milioni di cicli
- Veloce (ms)
- No bounce

**Svantaggi SSR**:
- Più costoso
- Genera calore
- Perdite in conduzione

---

## Configurazione Software

### Isteresi

Controlla la "banda morta" attorno al setpoint.

**Isteresi piccola (0.1-0.5°C)**:
- ✅ Temperatura più stabile
- ❌ Cicli frequenti relé
- ❌ Usura relé

**Isteresi grande (1-2°C)**:
- ✅ Meno cicli relé
- ✅ Maggiore durata
- ❌ Temperatura meno stabile

**Raccomandazioni**:
```cpp
// Incubatrice (precisione critica)
relayController.setHysteresis(0.3);

// Riscaldamento ambiente (confort)
relayController.setHysteresis(1.0);

// Forno (inerzia termica alta)
relayController.setHysteresis(2.0);
```

### Tempo Minimo Ciclo

Previene switching troppo rapido.

```cpp
relayController.setMinCycleTime(1000); // 1 secondo minimo
```

**Valori tipici**:
- Riscaldatori elettrici: 500-1000ms
- Compressori: 3000-5000ms (protezione)
- Ventole: 200-500ms
- SSR: 100ms

**Protezione Compressori**:
```cpp
// Compressori frigo: SEMPRE >= 3 minuti!
relayController.setMinCycleTime(180000); // 3 minuti
```

---

## Time Proportioning

Migliora il controllo usando PWM lento (cicli lunghi).

### Concetto

Invece di ON/OFF netto, il relé si accende per una frazione del periodo.

**Esempio**:
```
Output PID: 128/255 (50%)
Window: 10 secondi

Relé ON per 5 secondi
Relé OFF per 5 secondi
Ripeti...
```

### Vantaggi

- ✅ Controllo più fine
- ✅ Migliore stabilità temperatura
- ✅ Usa PID output completo (non solo ON/OFF)
- ✅ Meno overshoot

### Configurazione

```cpp
// Attiva time proportioning con finestra 10s
relayController.enableTimeProportioning(true, 10000);
```

**Window size** (periodo):
- 5-10s: Riscaldatori piccoli
- 10-30s: Riscaldatori grandi
- 30-60s: Sistemi con alta inerzia

### Esempio Completo

```cpp
// Setup
relayController.setMode(RELAY_HEATING_ONLY);
relayController.enableTimeProportioning(true, 10000);

// Nel loop, PID fornisce 0-255
double pidOutput = pid.compute(temperature);

// Relay controller converte in duty cycle
relayController.update(temperature, setpoint, pidOutput);
```

**Risultato**:
```
PID Output: 200/255 (78%)
Window: 10s

Relé ON per 7.8 secondi
Relé OFF per 2.2 secondi
```

---

## Applicazioni Pratiche

### Incubatrice Uova

```cpp
// Setup
relayController.setMode(RELAY_HEATING_ONLY);
relayController.setHysteresis(0.2); // Precisione alta
relayController.setMinCycleTime(1000);

// PID settings
pid.setTunings(10.0, 5.0, 2.0);
pid.setSetpoint(37.5); // Temperatura incubazione
```

**Componenti**:
- Relé 1: Lampada riscaldante 100W
- Sensore: DHT22 o DS18B20
- Setpoint: 37.5°C ±0.2°C

---

### Fermentatore Birra (Dual Mode)

```cpp
// Setup dual mode
relayController.setMode(RELAY_DUAL_MODE);
relayController.setHysteresis(0.5);
relayController.setMinCycleTime(2000);

// PID conservativo
pid.setTunings(5.0, 2.0, 1.0);
pid.setSetpoint(20.0); // Fermentazione Ale
```

**Componenti**:
- Relé 1: Heating pad 25W
- Relé 2: Ventola 12V o Peltier
- Sensore: DS18B20 waterproof
- Setpoint: 18-22°C

---

### Forno Reflow

```cpp
// Setup con time proportioning
relayController.setMode(RELAY_HEATING_ONLY);
relayController.enableTimeProportioning(true, 5000);
relayController.setMinCycleTime(500);

// PID aggressivo
pid.setTunings(15.0, 3.0, 8.0);

// Profilo temperatura
setReflowProfile();
```

**Componenti**:
- Relé 1: SSR → Elemento riscaldante 1500W
- Sensore: MAX31855 con termocoppia K
- Setpoint: Variabile secondo profilo (150-235°C)

**Profilo tipico**:
```cpp
void setReflowProfile() {
    // Preheat
    pid.setSetpoint(150);
    delay(60000);

    // Soak
    pid.setSetpoint(180);
    delay(90000);

    // Reflow
    pid.setSetpoint(235);
    delay(30000);

    // Cool down
    relayController.setMode(RELAY_OFF);
}
```

---

### Controllo Frigo

```cpp
// IMPORTANTE: Protezione compressore!
relayController.setMode(RELAY_COOLING_ONLY);
relayController.setHysteresis(1.0);
relayController.setMinCycleTime(180000); // 3 minuti MINIMO

// PID conservativo
pid.setTunings(3.0, 0.5, 1.0);
pid.setSetpoint(4.0); // Temperatura frigo
```

**⚠️ ATTENZIONE COMPRESSORI**:
- Mai riavviare prima di 3 minuti
- Isteresi >= 1°C
- Considera cicli di sbrinamento

---

## Controllo Manuale Relé

Per test o applicazioni speciali:

```cpp
// Attiva relé specifico
relayController.setRelay(1, true);  // Relé 1 ON
relayController.setRelay(2, false); // Relé 2 OFF

// Leggi stato
bool heating = relayController.getRelayState(1);
bool cooling = relayController.getRelayState(2);
```

**Via Web**:
```json
{
  "cmd": "setRelay",
  "relay": 1,
  "state": true
}
```

---

## Troubleshooting

### Relé non scatta

- ✓ Verificare alimentazione modulo (5V)
- ✓ Controllare pin GPIO corretti
- ✓ Testare con comando manuale
- ✓ LED su modulo si accende? (logica OK, problema relé)
- ✓ Modulo difettoso?

### Relé scatta continuamente

- ✓ Aumentare isteresi
- ✓ Verificare minCycleTime
- ✓ PID troppo aggressivo? (ridurre Kp)
- ✓ Sensore rumoroso? (filtrare)

### Carico non si attiva

- ✓ Usare NO (normally open) non NC
- ✓ Verificare cablaggio carico
- ✓ Fusibile integro?
- ✓ Carico funzionante? (testare diretto)
- ✓ Relé rated per corrente carico?

### Temperatura instabile

- ✓ Inerzia termica: aumentare integral (Ki)
- ✓ Usare time proportioning
- ✓ Aumentare window size
- ✓ Migliorare isolamento termico

### Relé si danneggia rapidamente

- ✓ Corrente carico troppo alta
- ✓ Cicli troppo frequenti (aumentare isteresi)
- ✓ Carichi induttivi (motori): aggiungere snubber
- ✓ Passare a SSR

---

## Sicurezza

### ⚠️ ALTA TENSIONE

**Regole ferree**:
1. ❌ Mai lavorare con alimentazione inserita
2. ✅ Usare fusibili
3. ✅ Cavi adeguati alla corrente
4. ✅ Morsetti ben serrati
5. ✅ Protezione meccanica (box)
6. ✅ Messa a terra
7. ✅ Ventilazione adeguata (SSR)

### Carichi Specifici

**Compressori**:
- Tempo minimo tra cicli: 3-5 minuti
- Relé rated 2x corrente nominale
- Soft start se disponibile

**Elementi riscaldanti**:
- Fusibile termico
- Protezione sovratemperatura
- Ventilazione

**Motori AC**:
- Snubber RC su contatti
- Relé rated per carichi induttivi

---

## Esempi Codice

### Setup Completo Fermentatore

```cpp
#include "RelayController.h"
#include "PIDController.h"
#include "SensorManager.h"

RelayController relay;
PIDController pid;
SensorManager sensors;

void setup() {
    // Inizializza sensore
    sensors.begin();
    sensors.setActiveSensor(SENSOR_DS18B20);

    // Setup PID
    pid.begin(5.0, 2.0, 1.0, 20.0); // Kp, Ki, Kd, Setpoint
    pid.setOutputLimits(0, 255);

    // Setup relé
    relay.begin();
    relay.setMode(RELAY_DUAL_MODE);
    relay.setHysteresis(0.5);
    relay.setMinCycleTime(2000);
}

void loop() {
    // Leggi temperatura
    sensors.update();
    float temp = sensors.getTemperature();

    // Calcola PID
    double output = pid.compute(temp);

    // Aggiorna relé
    relay.update(temp, pid.getSetpoint(), output);

    delay(100);
}
```

---

## Risorse

- [Relay Basics](https://www.electronics-tutorials.ws/io/io_5.html)
- [SSR Guide](https://www.crydom.com/en/tech-resources/application-notes/)
- [PID Tuning for Relay Control](https://www.eurotherm.com/pid-control/)
- [Compressor Protection](https://www.achrnews.com/articles/87543)
