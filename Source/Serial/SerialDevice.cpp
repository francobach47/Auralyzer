#include "SerialDevice.h"

#define kBPS 115200
const auto kNumberOfDecimalPlaces{ 4 };

enum ParseState
{
	waitingForStartByte1,
	waitingForStartByte2,
	waitingForCommand,
	waitingForCommandDataSize,
	waitingForCommandData
};
ParseState parseState = waitingForStartByte1;

const int kStartByte1 = '*';
const int kStartByte2 = '~';

enum Command
{
	none,
	lightColor,
    setMode,
    setRange,
    endOfList
};
const int kMaxPayloadSize = 20;
Command gTestCommandToExecute{ Command::lightColor };

SerialDevice::SerialDevice()
	: Thread(juce::String("SerialDevice"))
{
	// start the serial thread reading data
	startThread();

	// NOTE: this timer is used to send commands for testing purposes
	//startTimer(10);
}

SerialDevice::~SerialDevice()
{
	stopThread(500);
	closeSerialPort();
}

void SerialDevice::init(juce::String newSerialPortName)
{
	serialPortName = newSerialPortName;
}

void SerialDevice::open(void)
{
	if (serialPortName.length() > 0)
		threadTask = ThreadTask::openSerialPort;
}

void SerialDevice::close(void)
{
	threadTask = ThreadTask::closeSerialPort;
}

void SerialDevice::setLightColor(uint16_t color)
{
	if (serialPortOutput.get() == nullptr)
		return;

	const std::vector<uint8_t> data{ 
        kStartByte1, kStartByte2, 
        Command::lightColor, 2,
        static_cast<uint8_t>(color & 0xff), 
        static_cast<uint8_t>((color >> 8) & 0xff) };

	serialPortOutput->write(data.data(), data.size());
}

void SerialDevice::setMode(uint8_t mode)
{
    if (serialPortOutput == nullptr) 
        return;

    const std::vector<uint8_t> data{
        kStartByte1, kStartByte2,
        Command::setMode, 1,
        mode                     // 0 = AC, 1 = DC
    };
    serialPortOutput->write(data.data(), data.size());
}

void SerialDevice::setRange(uint8_t idx)
{
    jassert(idx < 4);
    if (serialPortOutput == nullptr) 
        return;

    const std::vector<uint8_t> data{
        kStartByte1, kStartByte2,
        Command::setRange, 1,
        idx                      // 0-3
    };
    serialPortOutput->write(data.data(), data.size());
}

bool SerialDevice::openSerialPort(void)
{
	serialPort = std::make_unique<SerialPort>([](juce::String, juce::String) {});
	bool opened = serialPort->open(serialPortName);

	if (opened)
	{
		SerialPortConfig serialPortConfig;
		serialPort->getConfig(serialPortConfig);
		serialPortConfig.bps = kBPS;
		serialPortConfig.databits = 8;
		serialPortConfig.parity = SerialPortConfig::SERIALPORT_PARITY_NONE;
		serialPortConfig.stopbits = SerialPortConfig::STOPBITS_1;
		serialPort->setConfig(serialPortConfig);

		serialPortInput = std::make_unique<SerialPortInputStream>(serialPort.get());
		serialPortOutput = std::make_unique<SerialPortOutputStream>(serialPort.get());
		juce::Logger::outputDebugString("Serial port: " + serialPortName + " opened");
	}
	else
	{
		// report error
		juce::Logger::outputDebugString("Unable to open serial port:" + serialPortName);
	}

	return opened;
}

void SerialDevice::closeSerialPort(void)
{
	serialPortOutput = nullptr;
	serialPortInput = nullptr;
	if (serialPort != nullptr)
	{
		serialPort->close();
		serialPort = nullptr;
	}
}

void SerialDevice::handleLightColorCommand(uint8_t* data, int dataSize)
{
    if (dataSize != 2)
        return;
    lightColor = static_cast<uint16_t>(data[0] + (data[1] << 8));
}

void SerialDevice::handleCommand(uint8_t command, uint8_t* data, int dataSize)
{
	switch (command)
	{
	case Command::lightColor: handleLightColorCommand(data, dataSize); break;
	}
}

#define kSerialPortBufferLen 256
void SerialDevice::run()
{
    const int kMaxCommandDataBytes = 4;
    uint8_t commandData[kMaxCommandDataBytes];
    uint8_t command = Command::none;
    uint8_t commandDataSize = 0;
    uint8_t commandDataCount = 0;
    while (!threadShouldExit())
    {
        switch (threadTask)
        {
        case ThreadTask::idle:
        {
            wait(1);
            if (serialPortName.isNotEmpty())
                threadTask = ThreadTask::openSerialPort;
        }
        break;
        case ThreadTask::openSerialPort:
        {
            if (openSerialPort())
            {
                threadTask = ThreadTask::processSerialPort;
            }
            else
            {
                threadTask = ThreadTask::delayBeforeOpening;
                delayStartTime = static_cast<uint64_t>(juce::Time::getMillisecondCounterHiRes());
            }
        }
        break;

        case ThreadTask::delayBeforeOpening:
        {
            wait(1);
            if (juce::Time::getApproximateMillisecondCounter() > delayStartTime + 1000)
                threadTask = ThreadTask::openSerialPort;
        }
        break;

        case ThreadTask::closeSerialPort:
        {
            closeSerialPort();
            threadTask = ThreadTask::idle;
        }
        break;

        case ThreadTask::processSerialPort:
        {
            // handle reading from the serial port
            if ((serialPortInput != nullptr) && (!serialPortInput->isExhausted()))
            {
                auto  bytesRead = 0;
                auto  dataIndex = 0;
                uint8_t incomingData[kSerialPortBufferLen];

                bytesRead = serialPortInput->read(incomingData, kSerialPortBufferLen);
                if (bytesRead < 1)
                {
                    wait(1);
                    continue;
                }
                else
                {
                    // TODO: extract into function or class
                    // parseIncomingData (incomingData, bytesRead);
                    auto resetParser = [this, &command, &commandDataSize, &commandDataCount]()
                        {
                            command = Command::none;
                            commandDataSize = 0;
                            commandDataCount = 0;
                            parseState = ParseState::waitingForStartByte1;
                        };
                    // parse incoming data
                    while (dataIndex < bytesRead)
                    {
                        const uint8_t dataByte = incomingData[dataIndex];
                        // NOTE: the following line prints each byte received in the debug output window
                        juce::Logger::outputDebugString(juce::String(dataByte));
                        switch (parseState)
                        {
                        case ParseState::waitingForStartByte1:
                        {
                            if (dataByte == kStartByte1)
                                parseState = ParseState::waitingForStartByte2;
                        }
                        break;
                        case ParseState::waitingForStartByte2:
                        {
                            if (dataByte == kStartByte2)
                                parseState = ParseState::waitingForCommand;
                            else
                                resetParser();
                        }
                        break;
                        case ParseState::waitingForCommand:
                        {
                            if (dataByte >= Command::none && dataByte < Command::endOfList)
                            {
                                command = dataByte;
                                parseState = ParseState::waitingForCommandDataSize;
                            }
                            else
                            {
                                resetParser();
                            }
                        }
                        break;
                        case ParseState::waitingForCommandDataSize:
                        {
                            // NOTE: this is only a one byte data length, it you want two bytes, you will need to add two states
                            //       to read in the length, one for LSB, and one for MSB
                            if (dataByte >= 0 && dataByte <= kMaxCommandDataBytes)
                            {
                                commandDataSize = dataByte;
                                parseState = ParseState::waitingForCommandData;
                            }
                            else
                            {
                                resetParser();
                            }
                        }
                        break;
                        case ParseState::waitingForCommandData:
                        {
                            if (commandDataSize != 0)
                            {
                                commandData[commandDataCount] = dataByte;
                                ++commandDataCount;
                            }
                            if (commandDataCount == commandDataSize)
                            {
                                // execute command
                                handleCommand(command, commandData, commandDataSize);

                                // reset and wait for next command packet
                                resetParser();
                            }
                        }
                        break;

                        default:
                        {
                            // unknown parse state, broken
                            jassertfalse;
                        }
                        }
                        ++dataIndex;
                    }
                }
            }
        }
        break;
        }
    }
}

void SerialDevice::timerCallback()
{

}
