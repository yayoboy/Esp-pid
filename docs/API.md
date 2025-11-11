# API Reference

Documentazione completa delle API REST e WebSocket del controller PID ESP32.

## Indice
- [REST API](#rest-api)
- [WebSocket API](#websocket-api)
- [Esempi Client](#esempi-client)

---

## REST API

### Base URL

```
http://[ESP32_IP_ADDRESS]/
```

### Endpoints

#### GET `/`

Restituisce l'interfaccia web principale (HTML).

**Response**: HTML page

---

#### GET `/api/status`

Ottiene lo stato completo del sistema.

**Response**:
```json
{
  "type": "status",
  "setpoint": 25.0,
  "input": 24.5,
  "output": 128.0,
  "mode": true,
  "kp": 2.0,
  "ki": 5.0,
  "kd": 1.0,
  "temp": 24.5,
  "humidity": 45.2,
  "position": 100
}
```

**Campi**:
- `setpoint` (float): Valore target del PID
- `input` (float): Valore corrente del processo
- `output` (float): Output del PID (0-255)
- `mode` (boolean): true=AUTO, false=MANUAL
- `kp`, `ki`, `kd` (float): Parametri PID
- `temp` (float): Temperatura in °C
- `humidity` (float): Umidità relativa %
- `position` (int): Posizione encoder

**Esempio**:
```bash
curl http://192.168.1.100/api/status
```

---

#### GET `/api/config`

Ottiene la configurazione PID corrente.

**Response**:
```json
{
  "kp": 2.0,
  "ki": 5.0,
  "kd": 1.0,
  "setpoint": 25.0
}
```

**Esempio**:
```bash
curl http://192.168.1.100/api/config
```

---

## WebSocket API

### Connessione

**URL**: `ws://[ESP32_IP_ADDRESS]/ws`

**Protocollo**: JSON text messages

### Eventi

#### Connessione

All'apertura della connessione WebSocket, il server invia automaticamente un messaggio di stato.

#### Messaggi dal Server

Il server invia periodicamente (ogni secondo) aggiornamenti di stato:

```json
{
  "type": "status",
  "setpoint": 25.0,
  "input": 24.5,
  "output": 128.0,
  "mode": true,
  "kp": 2.0,
  "ki": 5.0,
  "kd": 1.0,
  "temp": 24.5,
  "humidity": 45.2,
  "position": 100
}
```

#### Messaggi al Server

Il client può inviare comandi JSON per controllare il PID:

##### Comando: `setPID`

Imposta i parametri PID.

**Richiesta**:
```json
{
  "cmd": "setPID",
  "kp": 3.0,
  "ki": 4.0,
  "kd": 1.5
}
```

**Campi**:
- `kp` (float): Guadagno proporzionale
- `ki` (float): Guadagno integrale
- `kd` (float): Guadagno derivativo

**Response**: Messaggio di stato aggiornato

---

##### Comando: `setSetpoint`

Imposta il valore target (setpoint) del PID.

**Richiesta**:
```json
{
  "cmd": "setSetpoint",
  "value": 30.0
}
```

**Campi**:
- `value` (float): Nuovo setpoint

**Response**: Messaggio di stato aggiornato

---

##### Comando: `setMode`

Cambia modalità operativa del PID.

**Richiesta**:
```json
{
  "cmd": "setMode",
  "value": true
}
```

**Campi**:
- `value` (boolean): true=AUTO, false=MANUAL

**Response**: Messaggio di stato aggiornato

---

## Esempi Client

### JavaScript (Browser)

#### Connessione WebSocket

```javascript
// Connessione
const ws = new WebSocket('ws://192.168.1.100/ws');

// Gestione connessione
ws.onopen = function(event) {
    console.log('WebSocket connesso');
};

// Ricezione messaggi
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    console.log('Stato ricevuto:', data);

    // Aggiorna UI
    document.getElementById('temperature').textContent = data.temp;
    document.getElementById('output').textContent = data.output;
};

// Gestione errori
ws.onerror = function(error) {
    console.error('WebSocket errore:', error);
};

// Disconnessione
ws.onclose = function(event) {
    console.log('WebSocket disconnesso');
};
```

#### Invio Comandi

```javascript
// Imposta setpoint
function setSetpoint(value) {
    const cmd = {
        cmd: 'setSetpoint',
        value: parseFloat(value)
    };
    ws.send(JSON.stringify(cmd));
}

// Imposta parametri PID
function setPID(kp, ki, kd) {
    const cmd = {
        cmd: 'setPID',
        kp: parseFloat(kp),
        ki: parseFloat(ki),
        kd: parseFloat(kd)
    };
    ws.send(JSON.stringify(cmd));
}

// Cambia modalità
function setAutoMode(enabled) {
    const cmd = {
        cmd: 'setMode',
        value: enabled
    };
    ws.send(JSON.stringify(cmd));
}

// Esempi di utilizzo
setSetpoint(25.5);
setPID(2.0, 5.0, 1.0);
setAutoMode(true);
```

---

### Python

#### Richieste HTTP

```python
import requests
import json

ESP32_IP = "192.168.1.100"

# Ottieni stato
response = requests.get(f"http://{ESP32_IP}/api/status")
status = response.json()
print(f"Temperatura: {status['temp']}°C")
print(f"Output: {status['output']}")

# Ottieni configurazione
response = requests.get(f"http://{ESP32_IP}/api/config")
config = response.json()
print(f"Kp: {config['kp']}, Ki: {config['ki']}, Kd: {config['kd']}")
```

#### WebSocket Client

```python
import asyncio
import websockets
import json

async def pid_client():
    uri = "ws://192.168.1.100/ws"

    async with websockets.connect(uri) as websocket:
        print("Connesso al controller PID")

        # Imposta setpoint
        cmd = {
            "cmd": "setSetpoint",
            "value": 25.0
        }
        await websocket.send(json.dumps(cmd))

        # Ricevi messaggi
        while True:
            message = await websocket.recv()
            data = json.loads(message)
            print(f"Temp: {data['temp']:.1f}°C, Output: {data['output']:.0f}")

# Esegui
asyncio.run(pid_client())
```

#### Monitor Continuo

```python
import requests
import time
from datetime import datetime

ESP32_IP = "192.168.1.100"

def monitor_pid(interval=1.0):
    """Monitora il PID e salva log"""

    print("Time,Setpoint,Input,Output,Temp,Humidity")

    while True:
        try:
            response = requests.get(f"http://{ESP32_IP}/api/status", timeout=2)
            data = response.json()

            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            log_line = (f"{timestamp},"
                       f"{data['setpoint']},"
                       f"{data['input']},"
                       f"{data['output']},"
                       f"{data['temp']},"
                       f"{data['humidity']}")

            print(log_line)

            # Salva su file
            with open('pid_log.csv', 'a') as f:
                f.write(log_line + '\n')

        except Exception as e:
            print(f"Errore: {e}")

        time.sleep(interval)

# Esegui monitor
monitor_pid(interval=1.0)
```

---

### Node.js

```javascript
const WebSocket = require('ws');

const ESP32_IP = '192.168.1.100';
const ws = new WebSocket(`ws://${ESP32_IP}/ws`);

ws.on('open', function open() {
    console.log('Connesso al controller PID');

    // Imposta parametri PID
    ws.send(JSON.stringify({
        cmd: 'setPID',
        kp: 2.5,
        ki: 4.0,
        kd: 1.2
    }));
});

ws.on('message', function message(data) {
    const status = JSON.parse(data);
    console.log(`Temp: ${status.temp.toFixed(1)}°C, Output: ${status.output.toFixed(0)}`);
});

ws.on('error', function error(err) {
    console.error('WebSocket errore:', err);
});
```

---

### Arduino/ESP32 Client

Un altro ESP32 può controllare il PID:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ESP32_IP = "192.168.1.100";

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID", "PASSWORD");

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void loop() {
    HTTPClient http;

    // Ottieni stato
    http.begin(String("http://") + ESP32_IP + "/api/status");
    int httpCode = http.GET();

    if (httpCode == 200) {
        String payload = http.getString();

        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        float temp = doc["temp"];
        float output = doc["output"];

        Serial.printf("Temperatura: %.1f°C, Output: %.0f\n", temp, output);
    }

    http.end();
    delay(1000);
}
```

---

## Rate Limiting

Non ci sono limiti espliciti, ma si raccomanda:

- **REST API**: Max 10 richieste/secondo
- **WebSocket**: Comandi max 5/secondo
- Updates dal server: 1/secondo

---

## Error Handling

### HTTP Errors

- `404 Not Found`: Endpoint inesistente
- `500 Internal Server Error`: Errore server

### WebSocket Errors

In caso di comando invalido, il server ignora silenziosamente il messaggio.

**Validazione lato client**:
```javascript
function validatePID(kp, ki, kd) {
    if (kp < 0 || ki < 0 || kd < 0) {
        throw new Error("Parametri PID devono essere positivi");
    }
}
```

---

## Security

⚠️ **IMPORTANTE**: Il sistema NON ha autenticazione!

Per ambienti di produzione:
1. Implementare autenticazione HTTP Basic
2. Usare HTTPS/WSS (TLS)
3. Firewall per limitare accesso
4. VPN per accesso remoto

---

## CORS

Il server accetta richieste da qualsiasi origine (CORS abilitato).

Per limitare:
```cpp
// In WebServer.cpp
server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
    // Solo da origine specifica
    request->send(200, "application/json", getStatusJSON());
});
```

---

## Changelog

- **v1.0**: API iniziale
- **v1.1**: Aggiunto endpoint `/api/config`
- **v1.2**: WebSocket ottimizzato

---

## Support

Per bug o richieste, apri una issue su GitHub.
