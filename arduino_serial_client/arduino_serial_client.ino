#include <HardwareSerial.h>

// Definición del pin del LED - CAMBIA ESTO AL PIN QUE USES EN TU ESP32
const int LED_PIN = 2;  // Pin donde tienes conectado el LED

// Estados para el parsing de comandos
enum ParseState {
  waitingForStartByte1,
  waitingForStartByte2,
  waitingForCommand,
  waitingForCommandDataSize,
  waitingForCommandData
};
ParseState parseState = waitingForStartByte1;

// Bytes de inicio y comandos
const int kStartByte1 = '*';
const int kStartByte2 = '~';
enum Command {
    none,
    lightColor,
    tempo,
    chargingAlarmLevel,
    endOfList
};

// Buffer para datos seriales
const int kMaxPayloadSize = 100;
uint8_t gSerialDataBuffer[kMaxPayloadSize];

// Variables para el comando actual
const int kMaxCommandDataBytes = 4;
uint8_t gCommandData[kMaxCommandDataBytes];
uint8_t gCommand = Command::none;
uint8_t gCommandDataSize = 0;
uint8_t gCommandDataCount = 0;

// Estado del LED
bool currentLedState = false;

void handleLightColorCommand(uint8_t* commandData, const int commandDataSize) {
    if (commandDataSize != 2) return;
    
    uint16_t colorValue = (commandData[1] << 8) | commandData[0];
    bool newLedState = (colorValue > 0);
    
    if(newLedState != currentLedState) {
        currentLedState = newLedState;
        digitalWrite(LED_PIN, currentLedState ? HIGH : LOW);
        
        Serial.print("LED cambiado a: ");
        Serial.println(currentLedState ? "ON" : "OFF");
    }
}

void processCommand(uint8_t command, uint8_t* commandData, const int commandDataSize) {
    switch (command) {
        case Command::lightColor: 
            handleLightColorCommand(commandData, commandDataSize); 
            break;
        // Puedes añadir más comandos aquí si los necesitas
        default: 
            break;
    }
}

void parseInputData(const uint8_t* data, const int dataSize) {
    auto resetParseState = []() {
        gCommand = Command::none;
        gCommandDataSize = 0;
        gCommandDataCount = 0;
        parseState = ParseState::waitingForStartByte1;
    };

    for (int dataIndex = 0; dataIndex < dataSize; ++dataIndex) {
        const uint8_t dataByte = data[dataIndex];
        
        switch(parseState) {
            case ParseState::waitingForStartByte1:
                if (dataByte == kStartByte1)
                    parseState = ParseState::waitingForStartByte2;
                break;
                
            case ParseState::waitingForStartByte2:
                if (dataByte == kStartByte2)
                    parseState = ParseState::waitingForCommand;
                else
                    resetParseState();
                break;
                
            case ParseState::waitingForCommand:
                if (dataByte >= Command::none && dataByte < Command::endOfList) {
                    gCommand = dataByte;
                    parseState = ParseState::waitingForCommandDataSize;
                } else {
                    resetParseState();
                }
                break;
                
            case ParseState::waitingForCommandDataSize:
                if (dataByte >= 0 && dataByte <= kMaxCommandDataBytes) {
                    gCommandDataSize = dataByte;
                    parseState = ParseState::waitingForCommandData;
                } else {
                    resetParseState();
                }
                break;
                
            case ParseState::waitingForCommandData:
                if (gCommandDataSize != 0) {
                    gCommandData[gCommandDataCount] = dataByte;
                    ++gCommandDataCount;
                }
                if (gCommandDataCount == gCommandDataSize) {
                    processCommand(gCommand, gCommandData, gCommandDataSize);
                    resetParseState();
                }
                break;
                
            default:
                resetParseState();
        }
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("ESP32 Listo. Esperando comandos...");
}

void loop() {
    if (Serial.available() > 0) {
        uint8_t bytesToRead = min(Serial.available(), kMaxPayloadSize);
        Serial.readBytes(gSerialDataBuffer, bytesToRead);
        parseInputData(gSerialDataBuffer, bytesToRead);
    }
    delay(10); // Pequeña pausa para evitar sobrecarga
}
