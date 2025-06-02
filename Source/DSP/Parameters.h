#pragma once

#include <JuceHeader.h>

// Paramters ID
const juce::ParameterID horizontalPositionParamID{ "horizontalPosition", 1 };
const juce::ParameterID horizontalScaleParamID{ "horizontalScale", 1 };
const juce::ParameterID verticalPositionParamID{ "verticalPosition", 1 };
const juce::ParameterID verticalScaleParamID{ "verticalScale", 1 };
const juce::ParameterID rangeParamID{ "range", 1 };
const juce::ParameterID modeParamID{ "mode", 1 };
const juce::ParameterID plotModeParamID{ "plotMode", 1 };
const juce::ParameterID bypassParamID{ "bypass", 1 };
const juce::ParameterID triggerLevelParamID{ "triggerLevel", 1 };
const juce::ParameterID movingAverageParamID{ "movingAverage", 1 };
const std::vector<std::vector<std::pair<juce::String, float>>> verticalScaleByRange = {
	{ {"20 mV/div", 0.02f}, {"10 mV/div", 0.01f}, {"5 mV/div", 0.005f}, {"2 mV/div", 0.002f} },    // Rango 0
	{ {"200 mV/div", 0.2f}, {"100 mV/div", 0.1f}, {"50 mV/div", 0.05f}, {"20 mV/div", 0.02f} },    // Rango 1
	{ {"2 V/div", 2.0f}, {"1 V/div", 1.0f}, {"500 mV/div", 0.5f}, {"200 mV/div", 0.2f} },          // Rango 2
	{ {"20 V/div", 20.0f}, {"10 V/div", 10.0f}, {"5 V/div", 5.0f}, {"2 V/div", 2.0f} }             // Rango 3
};
inline const std::vector<std::pair<juce::String, float>> horizontalScaleOptions = {
	{ "10 s",   10.0f  }, { "5 s",    5.0f  }, { "2 s",    2.0f  }, { "1 s",    1.0f  },
	{ "500 ms", 0.5f  }, { "200 ms", 0.2f  }, { "100 ms", 0.1f  }, { "50 ms",  0.05f },
	{ "20 ms",  0.02f }, { "10 ms",  0.01f }, { "5 ms",   0.005f }, { "2 ms",   0.002f },
	{ "1 ms",   0.001f }, { "0.5 ms", 0.0005f }, { "0.2 ms", 0.0002f }, { "0.1 ms", 0.0001f }
};


class Parameters
{
public:
	Parameters(juce::AudioProcessorValueTreeState& apvts);

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void update() noexcept;

	// Parameters initial Values
	float horizontalPosition = 0.0f;
	int   horizontalScale    = 0;
	float verticalPosition   = 0.0f;
	int verticalScaleIndex   = 0;
	int	  rangeValue	     = 0;
	int   modeValue          = 0;

	juce::AudioParameterFloat* triggerLevelParam;
	float triggerLevel = 0.0f;
	
	int plotMode = 0;
	juce::AudioParameterChoice* plotModeParam;

	juce::AudioParameterChoice* rangeParam;

	juce::AudioParameterBool* movingAverageParam;
	bool movingAverage = true;

	float getTriggerLevel() const noexcept;
	float getVerticalScaleInVolts() const;
	float getHorizontalScaleInSeconds() const;
	int horizontalScaleIndex = 0;

private:

	// Parameters definition
	juce::AudioParameterFloat*  horizontalPositionParam;
	juce::AudioParameterChoice* horizontalScaleParam;
	juce::AudioParameterFloat*  verticalPositionParam;
	juce::AudioParameterChoice* verticalScaleParam;
	juce::AudioParameterChoice* modeParam;
};