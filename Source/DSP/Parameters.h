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
	int   verticalScale      = 0;
	int   rangeValue         = 0;
	int   modeValue          = 0;
	
	int plotMode = 0;
	juce::AudioParameterChoice* plotModeParam;

private:

	// Parameters definition
	juce::AudioParameterFloat*  horizontalPositionParam;
	juce::AudioParameterInt*    horizontalScaleParam;
	juce::AudioParameterFloat*  verticalPositionParam;
	juce::AudioParameterInt*    verticalScaleParam;
	juce::AudioParameterChoice* rangeParam;
	juce::AudioParameterChoice* modeParam;
};