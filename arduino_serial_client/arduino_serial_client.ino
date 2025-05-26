#include <HardwareSerial.h>

// ---------- Pines de salida -----------------------------------
const int LED_PIN  = 2;    // led testigo (sin cambios)
const int PIN_AC   = 25;   // MODO – AC
const int PIN_DC   = 26;   // MODO – DC
const int PIN_R0   = 27;   // RANGO 0
const int PIN_R1   = 32;   // RANGO 1
const int PIN_R2   = 33;   // RANGO 2
const int PIN_R3   = 14;   // RANGO 3
// --------------------------------------------------------------

// ---------- Parser y comandos (sin cambios salvo los nuevos) ---
enum ParseState {
  waitingForStartByte1,
  waitingForStartByte2,
  waitingForCommand,
  waitingForCommandDataSize,
  waitingForCommandData
};
ParseState parseState = waitingForStartByte1;

const int kStartByte1 = '*';
const int kStartByte2 = '~';

enum Command {
    none,
    lightColor,   // se mantiene para el LED
    setMode,   // NUEVO  (payload 1 byte: 0=AC,1=DC)
    setRange,   // NUEVO  (payload 1 byte: 0-3)
    // los antiguos 'tempo' y 'chargingAlarmLevel' no se usan ahora
    endOfList
};

const int kMaxPayloadSize      = 100;
uint8_t   gSerialDataBuffer[kMaxPayloadSize];

const int kMaxCommandDataBytes = 4;
uint8_t   gCommandData[kMaxCommandDataBytes];
uint8_t   gCommand          = Command::none;
uint8_t   gCommandDataSize  = 0;
uint8_t   gCommandDataCount = 0;

// ---------- Estado del LED interno -----------------------------
bool currentLedState = false;

// ---------- Helpers de salidas ---------------------------------
void activarModo(uint8_t m)               // 0 = AC, 1 = DC
{
    digitalWrite(PIN_AC, m == 0 ? HIGH : LOW);
    digitalWrite(PIN_DC, m == 1 ? HIGH : LOW);
}

void activarRango(uint8_t idx)            // 0-3
{
    digitalWrite(PIN_R0, LOW);
    digitalWrite(PIN_R1, LOW);
    digitalWrite(PIN_R2, LOW);
    digitalWrite(PIN_R3, LOW);

    switch (idx)
    {
        case 0: digitalWrite(PIN_R0, HIGH); break;
        case 1: digitalWrite(PIN_R1, HIGH); break;
        case 2: digitalWrite(PIN_R2, HIGH); break;
        case 3: digitalWrite(PIN_R3, HIGH); break;
    }
}

// ---------- Handlers de comandos -------------------------------
void handleLightColorCommand(uint8_t* commandData, const int sz)
{
    if (sz != 2) return;

    uint16_t colorValue = (commandData[1] << 8) | commandData[0];
    bool newLedState = (colorValue > 0);

    if (newLedState != currentLedState)
    {
        currentLedState = newLedState;
        digitalWrite(LED_PIN, currentLedState ? HIGH : LOW);

        Serial.print("LED cambiado a: ");
        Serial.println(currentLedState ? "ON" : "OFF");
    }
}

void handleSetMode(uint8_t* commandData, const int sz)
{
    if (sz != 1 || commandData[0] > 1) return;
    activarModo(commandData[0]);
}

void handleSetRange(uint8_t* commandData, const int sz)
{
    if (sz != 1 || commandData[0] > 3) return;
    activarRango(commandData[0]);
}

void processCommand(uint8_t command, uint8_t* data, const int sz)
{
    switch (command)
    {
        case Command::lightColor: handleLightColorCommand(data, sz); break;
        case Command::setMode   : handleSetMode        (data, sz);   break;
        case Command::setRange  : handleSetRange       (data, sz);   break;
        default: break;
    }
}

// ---------- Parser (sin cambios) -------------------------------
void parseInputData(const uint8_t* data, const int dataSize)
{
    auto resetParseState = []()
    {
        gCommand = Command::none;
        gCommandDataSize = 0;
        gCommandDataCount = 0;
        parseState = ParseState::waitingForStartByte1;
    };

    for (int i = 0; i < dataSize; ++i)
    {
        const uint8_t byteIn = data[i];

        switch (parseState)
        {
            case waitingForStartByte1:
                if (byteIn == kStartByte1) parseState = waitingForStartByte2;
                break;

            case waitingForStartByte2:
                if (byteIn == kStartByte2) parseState = waitingForCommand;
                else resetParseState();
                break;

            case waitingForCommand:
                if (byteIn < Command::endOfList)
                {
                    gCommand = byteIn;
                    parseState = waitingForCommandDataSize;
                }
                else resetParseState();
                break;

            case waitingForCommandDataSize:
                if (byteIn <= kMaxCommandDataBytes)
                {
                    gCommandDataSize = byteIn;
                    gCommandDataCount = 0;
                    parseState = waitingForCommandData;
                }
                else resetParseState();
                break;

            case waitingForCommandData:
                if (gCommandDataCount < gCommandDataSize)
                    gCommandData[gCommandDataCount++] = byteIn;

                if (gCommandDataCount == gCommandDataSize)
                {
                    processCommand(gCommand, gCommandData, gCommandDataSize);
                    resetParseState();
                }
                break;

            default: resetParseState();
        }
    }
}

// ---------- Setup / Loop ---------------------------------------
void setup()
{
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    pinMode(PIN_AC, OUTPUT);   pinMode(PIN_DC, OUTPUT);
    pinMode(PIN_R0, OUTPUT);   pinMode(PIN_R1, OUTPUT);
    pinMode(PIN_R2, OUTPUT);   pinMode(PIN_R3, OUTPUT);

    Serial.println("ESP32 listo. esperando comandos...");
}

void loop()
{
    if (Serial.available() > 0)
    {
        uint8_t n = min(Serial.available(), kMaxPayloadSize);
        Serial.readBytes(gSerialDataBuffer, n);
        parseInputData(gSerialDataBuffer, n);
    }
    delay(10);
}

