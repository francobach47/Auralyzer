#include <HardwareSerial.h>

// ---------- Pines de entrada/salida -----------------------------
const int LED_PIN      = 2;    // LED interno de la placa
const int CTRL_PIN     = 23;   // 1 = plugin controla, 0 = llaves

const int PIN_MODO     = 27;   // Salida para modo: HIGH = AC, LOW = DC

const int PIN_R0       = 32;   // Salidas AC
const int PIN_R1       = 33;
const int PIN_R2       = 25;
const int PIN_R3       = 26;

const int PIN_BIN0     = 12;   // Salidas DC (bits invertidos)
const int PIN_BIN1     = 14;

// Entradas para replicar desde llaves analógicas
const int IN_MODO      = 5;
const int IN_BIN0_i    = 4;
const int IN_BIN1_i    = 0;

// Estado previo
bool    lastControlState = true;   // HIGH=plugin controla
uint8_t lastModo         = 255;
uint8_t lastRango        = 255;

// ---------- Protocolo serial ------------------------------------
enum ParseState {
  waitingForStartByte1,
  waitingForStartByte2,
  waitingForCommand,
  waitingForCommandDataSize,
  waitingForCommandData
};
ParseState parseState = waitingForStartByte1;

const uint8_t kStartByte1 = '*';
const uint8_t kStartByte2 = '~';

enum Command {
  none,
  lightColor,
  setMode,
  setRange,
  syncKnobs,
  controlModeStatus,
  endOfList
};

const int kMaxPayloadSize      = 100;
const int kMaxCommandDataBytes = 4;

uint8_t gSerialDataBuffer[kMaxPayloadSize];
uint8_t gCommandData[kMaxCommandDataBytes];
uint8_t gCommand          = Command::none;
uint8_t gCommandDataSize  = 0;
uint8_t gCommandDataCount = 0;

// ---------- Helpers de activación ------------------------------
void activarModo(uint8_t m) {
  // m==0 → AC (HIGH), m==1 → DC (LOW)
  digitalWrite(PIN_MODO, m == 0 ? HIGH : LOW);
}

void activarRango(uint8_t idx) {
  // Limpio todas las salidas
  digitalWrite(PIN_R0, LOW);
  digitalWrite(PIN_R1, LOW);
  digitalWrite(PIN_R2, LOW);
  digitalWrite(PIN_R3, LOW);
  digitalWrite(PIN_BIN0, LOW);
  digitalWrite(PIN_BIN1, LOW);

  // Selección AC
  switch (idx) {
    case 0: digitalWrite(PIN_R0, HIGH); break;
    case 1: digitalWrite(PIN_R1, HIGH); break;
    case 2: digitalWrite(PIN_R2, HIGH); break;
    case 3: digitalWrite(PIN_R3, HIGH); break;
  }

  // Bits DC invertidos
  uint8_t inv = 3 - idx;
  digitalWrite(PIN_BIN1, (inv & 0b10) >> 1);
  digitalWrite(PIN_BIN0, (inv & 0b01));
}

// ---------- Envío de comandos al plugin -------------------------
void sendCommand(uint8_t cmd, const uint8_t* data, uint8_t size) {
  Serial.write(kStartByte1);
  Serial.write(kStartByte2);
  Serial.write(cmd);
  Serial.write(size);
  Serial.write(data, size);
}

void sendControlMode(bool pluginControls) {
  uint8_t payload = pluginControls ? 1 : 0;
  sendCommand(controlModeStatus, &payload, 1);
}

void sendSyncKnobs(uint8_t modo, uint8_t rango) {
  uint8_t payload[2] = { modo, rango };
  sendCommand(syncKnobs, payload, 2);
}

// ---------- Recepción de comandos del plugin --------------------
void handleSetMode(uint8_t* d, int sz) {
  if (sz == 1 && d[0] <= 1)
    activarModo(d[0]);
}

void handleSetRange(uint8_t* d, int sz) {
  if (sz == 1 && d[0] <= 3)
    activarRango(d[0]);
}

void processCommand(uint8_t c, uint8_t* d, int sz) {
  switch (c) {
    case setMode:  handleSetMode (d, sz); break;
    case setRange: handleSetRange(d, sz); break;
    default: break;
  }
}

void parseInputData(const uint8_t* data, int dataSize) {
  auto resetParser = [] {
    gCommand          = Command::none;
    gCommandDataSize  = 0;
    gCommandDataCount = 0;
    parseState        = waitingForStartByte1;
  };

  for (int i = 0; i < dataSize; ++i) {
    uint8_t b = data[i];
    switch (parseState) {
      case waitingForStartByte1:
        if (b == kStartByte1) parseState = waitingForStartByte2;
        break;

      case waitingForStartByte2:
        if (b == kStartByte2) parseState = waitingForCommand;
        else resetParser();
        break;

      case waitingForCommand:
        if (b < endOfList) {
          gCommand = b;
          parseState = waitingForCommandDataSize;
        } else resetParser();
        break;

      case waitingForCommandDataSize:
        if (b <= kMaxCommandDataBytes) {
          gCommandDataSize = b;
          gCommandDataCount = 0;
          parseState = waitingForCommandData;
        } else resetParser();
        break;

      case waitingForCommandData:
        gCommandData[gCommandDataCount++] = b;
        if (gCommandDataCount == gCommandDataSize) {
          processCommand(gCommand, gCommandData, gCommandDataSize);
          resetParser();
        }
        break;

      default:
        resetParser();
        break;
    }
  }
}

// ---------- Lectura de llaves analógicas -----------------------
uint8_t leerModo() {
  return digitalRead(IN_MODO) ? 0 : 1; // HIGH=AC(0), LOW=DC(1)
}

uint8_t leerRango() {
  uint8_t inv = (digitalRead(IN_BIN1_i) << 1) | digitalRead(IN_BIN0_i);
  return 3 - inv; // bits invertidos → rango 0–3
}

// ---------- Setup / Loop ----------------------------------------
void setup() {
  Serial.begin(115200);

  // Configuro pines
  pinMode(LED_PIN, OUTPUT);
  pinMode(CTRL_PIN, INPUT_PULLUP);  // activa pull-up interna

  pinMode(PIN_MODO, OUTPUT);
  pinMode(PIN_R0, OUTPUT);
  pinMode(PIN_R1, OUTPUT);
  pinMode(PIN_R2, OUTPUT);
  pinMode(PIN_R3, OUTPUT);
  pinMode(PIN_BIN0, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT);

  pinMode(IN_MODO,  INPUT_PULLUP);
  pinMode(IN_BIN0_i, INPUT_PULLUP);
  pinMode(IN_BIN1_i, INPUT_PULLUP);

  // Iniciales
  digitalWrite(LED_PIN, LOW);
  activarModo(0);
  activarRango(0);

  Serial.println("ESP32 listo.");
}

void loop() {
  // 1) ¿Cambió quién controla?
  bool pluginControls = digitalRead(CTRL_PIN); // HIGH=plugin controla
  if (pluginControls != lastControlState) {
    lastControlState = pluginControls;
    // invertimos el LED: ON si llaves controlan, OFF si plugin controla
    digitalWrite(LED_PIN, pluginControls ? LOW : HIGH);
    sendControlMode(pluginControls);
  }

  // 2) Si las llaves controlan (CTRL_PIN == LOW), envío sus valores
  if (!pluginControls) {
    uint8_t modoActual  = leerModo();
    uint8_t rangoActual = leerRango();
    if (modoActual != lastModo || rangoActual != lastRango) {
      lastModo  = modoActual;
      lastRango = rangoActual;
      sendSyncKnobs(modoActual, rangoActual);
    }
  }

  // 3) Procesar comandos entrantes del plugin
  if (Serial.available() > 0) {
    int n = min(Serial.available(), kMaxPayloadSize);
    Serial.readBytes(gSerialDataBuffer, n);
    parseInputData(gSerialDataBuffer, n);
  }

  delay(10);
}