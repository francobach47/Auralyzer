#pragma once

#include <JuceHeader.h>

const juce::ParameterID horizontalScaleParamID{ "horizontalScale", 1 };
const juce::ParameterID horizontalPositionParamID{ "horizontalPosition", 1 };
const juce::ParameterID verticalScaleParamID{ "verticalScale", 1 };
const juce::ParameterID verticalPositionParamID{ "verticalPosition", 1 };

const juce::ParameterID rangeParamID{ "range", 1 };
const juce::ParameterID modeParamID{ "mode", 1 };

const juce::ParameterID timeFreqParamID{ "timeFreq", 1 };

const juce::ParameterID bypassParamID{ "bypass", 1 };

class Parameters
{
public:
	Parameters(juce::AudioProcessorValueTreeState& apvts);

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void update() noexcept;

	float horizontalPosition, verticalPosition;
	int horizontalScale, verticalScale = 0;
	
	int modeValue, rangeValue = 0;

	bool freqTyme = false;

private:
	juce::AudioParameterInt* horizontalScaleParam;
	juce::AudioParameterFloat* horizontalPositionParam;

	juce::AudioParameterInt* verticalScaleParam;
	juce::AudioParameterFloat* verticalPositionParam;

	juce::AudioParameterChoice* rangeParam;
	juce::AudioParameterChoice* modeParam;
	
	juce::AudioParameterBool* timeFreqParam;
};