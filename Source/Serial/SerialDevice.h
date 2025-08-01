#pragma once

#include "SerialPortListMonitor.h"

class SerialDevice : private juce::Thread, private juce::Timer
{
public:
	SerialDevice();
	~SerialDevice();

	void open(void);
	void close(void);
	void init(juce::String newSerialPortName);

	void setLightColor(uint16_t color);
	void setCalibrationMode(uint8_t mode);

	void setMode(uint8_t mode);   
	void setRange(uint8_t idx);

	juce::String getCurrentPortName() const { return serialPortName; }

	SerialPortListMonitor serialPortListMonitor;
	std::function<void(uint8_t modo, uint8_t rango)> onSyncKnobsReceived;
	std::function<void(bool pluginShouldControl)> onControlStatusReceived;

private:
	enum class ThreadTask
	{
		idle,
		delayBeforeOpening,
		openSerialPort,
		closeSerialPort,
		processSerialPort,
	};

	juce::String serialPortName;
	std::unique_ptr<SerialPort> serialPort;
	std::unique_ptr<SerialPortInputStream> serialPortInput;
	std::unique_ptr<SerialPortOutputStream> serialPortOutput;

	ThreadTask threadTask{ ThreadTask::idle };
	uint64_t delayStartTime{ 0 };

	uint16_t lightColor{ 0 };
	uint16_t calibrationMode{ 0 };
	float tempo{ 60.0f };
	uint8_t alarmLevels[2]{ 0,0 };
	

	bool openSerialPort(void);
	void closeSerialPort(void);

	void handleLightColorCommand(uint8_t* data, int dataSize);
	void handleCommand(uint8_t command, uint8_t* data, int dataSize);
	void handleCalibrationMode(uint8_t* data, int dataSize);

	void run() override;
	void timerCallback() override;
};