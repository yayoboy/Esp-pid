# Guida Sensori Supportati

Documentazione completa per tutti i sensori supportati dal controller PID ESP32.

## Indice
- [Sensori di Temperatura](#sensori-di-temperatura)
- [Termocoppie](#termocoppie)
- [DS18B20](#ds18b20)
- [Calibrazione](#calibrazione)
- [Auto-Calibrazione](#auto-calibrazione)

---

## Sensori di Temperatura

Il sistema supporta diversi tipi di sensori di temperatura, ognuno con caratteristiche specifiche:

### DHT22 (AM2302)

**Caratteristiche**:
- Range: -40°C a +80°C
- Precisione: ±0.5°C
- Umidità: 0-100% RH (±2-5%)
- Interfaccia: 1-Wire digitale
- Tempo di risposta: ~2 secondi

**Collegamenti**:
```
ESP32 GPIO 16 ──> DHT22 DATA
3.3V          ──> VCC
GND           ──> GND
```

**Resistenza Pull-up**: 10kΩ tra DATA e VCC (opzionale, molti moduli l'hanno già integrata)

**Applicazioni ideali**:
- Monitoraggio ambientale
- Incubatrici
- Serre
- Controllo climatico generale

---

### BME280

**Caratteristiche**:
- Range temperatura: -40°C a +85°C
- Precisione: ±1.0°C
- Umidità: 0-100% RH (±3%)
- Pressione: 300-1100 hPa (±1 hPa)
- Interfaccia: I2C (0x76 o 0x77)
- Tempo di risposta: ~1 secondo

**Collegamenti**:
```
ESP32 GPIO 21 (SDA) ──> BME280 SDA
ESP32 GPIO 22 (SCL) ──> BME280 SCL
3.3V                ──> VCC
GND                 ──> GND
```

**Applicazioni ideali**:
- Stazioni meteo
- Monitoraggio ambientale professionale
- Sistemi HVAC
- Altimetria

---

## Termocoppie

Le termocoppie permettono misure ad alta temperatura, ideali per applicazioni industriali.

### MAX31855 (Raccomandato)

**Caratteristiche**:
- Range: -200°C a +1350°C
- Risoluzione: 0.25°C
- Precisione: ±2°C (fino a 700°C)
- Termocoppia: Tipo K
- Interfaccia: SPI
- Compensazione giunzione fredda: Integrata
- Rilevamento guasti: Sì (circuito aperto, cortocircuiti)

**Collegamenti**:
```
ESP32 GPIO 5  (CS)   ──> MAX31855 CS
ESP32 GPIO 18 (SCK)  ──> MAX31855 SCK
ESP32 GPIO 19 (MISO) ──> MAX31855 SO
3.3V                 ──> VCC
GND                  ──> GND
```

**Terminali Termocoppia**:
```
T+ (giallo)  ──> MAX31855 T+
T- (rosso)   ──> MAX31855 T-
```

**Note importanti**:
- Usare sempre termocoppia tipo K
- Rispettare la polarità (giallo=+, rosso=-)
- Cavi della termocoppia devono essere corti per ridurre rumore
- Non vicino a sorgenti di interferenza elettromagnetica

**Diagnostica errori**:
Il MAX31855 rileva automaticamente:
- Termocoppia non collegata (open circuit)
- Cortocircuito a GND
- Cortocircuito a VCC

**Applicazioni ideali**:
- Forni industriali
- Reflow oven (saldatura SMD)
- Tostatura caffè
- Cottura ceramica
- Temperature fino a 1000°C+

---

### MAX6675

**Caratteristiche**:
- Range: 0°C a +1024°C
- Risoluzione: 0.25°C
- Precisione: ±3°C
- Termocoppia: Tipo K
- Interfaccia: SPI
- Compensazione giunzione fredda: Integrata

**Collegamenti**:
```
ESP32 GPIO 5  (CS)   ──> MAX6675 CS
ESP32 GPIO 18 (SCK)  ──> MAX6675 SCK
ESP32 GPIO 19 (MISO) ──> MAX6675 SO
5V (o 3.3V)          ──> VCC
GND                  ──> GND
```

**Limitazioni**:
- Non misura temperature negative
- Meno preciso del MAX31855
- Nessun rilevamento guasti
- Più lento (lettura ogni 220ms)

**Note**:
Il MAX31855 è preferibile in quasi tutti i casi. Usare MAX6675 solo se già disponibile.

**Applicazioni ideali**:
- Progetti hobbistici alte temperature
- Sostituzione economica del MAX31855
- Temperature moderate (< 500°C)

---

## DS18B20

**Caratteristiche**:
- Range: -55°C a +125°C
- Risoluzione: 0.0625°C (12-bit)
- Precisione: ±0.5°C (-10°C a +85°C)
- Interfaccia: 1-Wire (Dallas)
- Alimentazione: 3.3V o 5V
- Waterproof: Versioni disponibili
- Multiple sensori: Sì, su stesso bus

**Collegamenti**:
```
ESP32 GPIO 26 ──> DS18B20 DATA (giallo)
3.3V          ──> VCC (rosso)
GND           ──> GND (nero)
```

**Resistenza Pull-up**: 4.7kΩ tra DATA e VCC (obbligatoria!)

**Configurazione Multi-Sensore**:
```
                    ┌───[4.7kΩ]───┐
                    │             │
ESP32 GPIO 26 ──────┼─────┬───────┴──── 3.3V
                    │     │
                DS18B20_1 │
                    │     │
                    │  DS18B20_2
                    │     │
                    │  DS18B20_3
                    │
                   GND
```

Ogni DS18B20 ha un indirizzo univoco di 64-bit, permettendo di collegare fino a 100+ sensori sullo stesso bus!

**Vantaggi**:
- Impermeabile (versioni con sonda)
- Lunghi cavi possibili (fino a 100m con cavo schermato)
- Multipli sensori su un solo pin
- Basso costo
- Precisione eccellente

**Svantaggi**:
- Tempo di conversione lento (~750ms a 12-bit)
- Range limitato rispetto alle termocoppie

**Applicazioni ideali**:
- Fermentazione birra/vino
- Acquari
- Piscine
- Misure multiple di temperatura
- Ambienti umidi o sommersi
- Distanze medie (fino a 100m)

---

## Comparazione Sensori

| Sensore      | Range (°C)      | Precisione | Costo | Velocità | Migliore per             |
|--------------|-----------------|------------|-------|----------|--------------------------|
| DHT22        | -40 a +80       | ±0.5°C     | €     | Lenta    | Ambiente, incubatrici    |
| BME280       | -40 a +85       | ±1.0°C     | €€    | Media    | Meteo, HVAC              |
| MAX31855     | -200 a +1350    | ±2.0°C     | €€€   | Veloce   | Forni, alte temperature  |
| MAX6675      | 0 a +1024       | ±3.0°C     | €€    | Lenta    | Hobbistica alte temp     |
| DS18B20      | -55 a +125      | ±0.5°C     | €     | Lenta    | Liquidi, multi-punto     |

---

## Calibrazione

Il sistema include un sofisticato sistema di calibrazione per compensare gli errori dei sensori.

### Concetti Base

**Offset**: Correzione additiva
```
ValoreCalbrato = ValoreGrezzo + Offset
```

**Scale**: Correzione moltiplicativa
```
ValoreCalibrato = ValoreGrezzo × Scale
```

**Formula Completa**:
```
ValoreCalibrato = (ValoreGrezzo × Scale) + Offset
```

### Calibrazione Manuale

**Via Seriale**:
```cpp
// Imposta offset di +2.5°C
sensors.setCalibration(2.5, 1.0);
sensors.enableCalibration(true);
```

**Via Web**:
Inviare comando WebSocket:
```json
{
  "cmd": "setCalibration",
  "offset": 2.5,
  "scale": 1.0
}
```

### Quando Calibrare

1. **Nuovo sensore**: Sempre controllare con riferimento noto
2. **Dopo molto tempo**: Sensori possono derivare
3. **Applicazioni critiche**: Dove precisione è essenziale
4. **Multiple misure**: Per omogeneizzare letture

### Metodo di Calibrazione Manuale

**Step 1**: Preparare riferimento
- Bagno d'acqua con termometro calibrato
- Camera climatica
- Punto di fusione ghiaccio (0°C)
- Punto di ebollizione acqua (100°C a livello del mare)

**Step 2**: Misurare errore
```
Riferimento: 25.0°C
Sensore:     23.5°C
Errore:      -1.5°C
```

**Step 3**: Applicare correzione
```
Offset = Riferimento - Sensore = 25.0 - 23.5 = +1.5°C
```

**Step 4**: Verificare
```
Sensore dopo calibrazione: 25.0°C ✓
```

---

## Auto-Calibrazione

Il sistema può auto-calibrarsi usando un valore di riferimento noto.

### Come Funziona

1. Inserire sensore in ambiente con temperatura nota
2. Avviare auto-calibrazione con valore di riferimento
3. Sistema raccoglie 100 campioni
4. Calcola media e offset automaticamente
5. Applica calibrazione

### Procedura

**Via Seriale**:
```cpp
// Avvia calibrazione a 25.0°C
sensors.startAutoCalibration(25.0);

// Attendi completamento (circa 100 secondi)

// Sistema applica automaticamente la calibrazione
```

**Via Web**:
```json
{
  "cmd": "startAutoCalibration",
  "reference": 25.0
}
```

**Log Output**:
```
========================================
Starting auto-calibration
Reference value: 25.00
Please wait for 100 samples...
========================================

Auto-calibration progress: 10%
Auto-calibration progress: 20%
...
Auto-calibration progress: 100%

========================================
Auto-calibration complete!
Samples collected: 100
Average reading: 23.45
Reference value: 25.00
Calculated offset: +1.55
========================================
```

### Punti di Calibrazione Comuni

**0°C - Ghiaccio fondente**:
```
- Bicchiere con ghiaccio tritato
- Aggiungere poca acqua
- Mescolare
- Attendere stabilizzazione
- Avviare calibrazione con riferimento 0.0°C
```

**100°C - Acqua bollente** (solo sensori adatti):
```
- Pentola d'acqua in ebollizione
- Correggere per altitudine (es. 98°C a 600m)
- Immergere sensore (attenzione!)
- Avviare calibrazione
```

**25°C - Temperatura ambiente controllata**:
```
- Stanza climatizzata
- Termometro di riferimento certificato
- Attendere equilibrio termico
- Avviare calibrazione
```

### Best Practices

1. **Ambiente stabile**: Attendere almeno 5 minuti di stabilizzazione
2. **Riferimento accurato**: Usare termometro certificato
3. **Evitare correnti**: Aria ferma durante calibrazione
4. **Multiple temperature**: Calibrare a 2-3 punti per linearità
5. **Ripetere**: Verificare periodicamente

---

## Selezione del Sensore Attivo

Il sistema rileva automaticamente tutti i sensori collegati e seleziona il primo disponibile.

### Priorità Automatica

1. DHT22
2. BME280
3. MAX31855
4. MAX6675
5. DS18B20
6. Encoder (fallback)

### Cambio Manuale

**Via Seriale**:
```cpp
sensors.setActiveSensor(SENSOR_MAX31855);
```

**Via Web**:
```json
{
  "cmd": "setSensor",
  "type": "MAX31855"
}
```

**Tipi disponibili**:
- `DHT22`
- `BME280`
- `MAX31855`
- `MAX6675`
- `DS18B20`
- `ENCODER`

---

## Troubleshooting

### Sensore non rilevato

**DHT22/BME280**:
- Verificare alimentazione 3.3V
- Controllare pull-up (4.7k-10kΩ)
- Testare con I2C scanner (BME280)

**Termocoppie**:
- Verificare polarità termocoppia
- Controllare tipo (deve essere K)
- SPI: verificare CS, SCK, MISO
- Attendere > 500ms dopo power-on

**DS18B20**:
- Resistenza pull-up 4.7kΩ OBBLIGATORIA
- Verificare indirizzo con scanner
- Cavo troppo lungo? (max 100m)

### Letture errate

- **Drift**: Ricalibrare sensore
- **Rumore**: Aggiungere condensatore 0.1µF
- **Interferenze**: Separare da cavi di potenza
- **Termiche**: Attendere stabilizzazione

### Letture instabili

- **Media mobile**: Attivare filtro software
- **Sample rate**: Ridurre frequenza lettura
- **Schermatura**: Usare cavi schermati
- **GND**: Verificare terra comune

---

## Esempi Applicazioni

### Forno Reflow (MAX31855)

```cpp
sensors.setActiveSensor(SENSOR_MAX31855);
pid.setSetpoint(235.0); // Temperatura picco
pid.setTunings(10.0, 2.0, 5.0); // PID aggressivo
```

### Fermentatore (DS18B20)

```cpp
sensors.setActiveSensor(SENSOR_DS18B20);
pid.setSetpoint(20.0); // Fermentazione ale
pid.setTunings(5.0, 1.0, 2.0); // PID moderato
```

### Incubatrice (DHT22)

```cpp
sensors.setActiveSensor(SENSOR_DHT22);
pid.setSetpoint(37.5); // Temperatura incubazione
pid.setTunings(2.0, 5.0, 1.0); // PID conservativo
// Monitorare anche umidità!
```

### Serra (BME280)

```cpp
sensors.setActiveSensor(SENSOR_BME280);
// Controllo multi-parametro
float temp = sensors.getTemperature();
float humidity = sensors.getHumidity();
float pressure = sensors.getPressure();
```

---

## Risorse

- [DHT22 Datasheet](https://www.sparkfun.com/datasheets/Sensors/Temperature/DHT22.pdf)
- [BME280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)
- [MAX31855 Datasheet](https://datasheets.maximintegrated.com/en/ds/MAX31855.pdf)
- [DS18B20 Datasheet](https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf)
- [Thermocouple Guide](https://www.omega.com/en-us/resources/thermocouples)
