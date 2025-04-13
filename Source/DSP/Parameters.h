#pragma once

#include <JuceHeader.h>

const juce::ParameterID bypassParamID{ "bypass", 1 };

class Parameters
{
public:
	Parameters(juce::AudioProcessorValueTreeState& apvts);

	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	//bool bypassed = false;
	//juce::AudioParameterBool* bypassParam;

private:
	// completar con los parametros
	//juce::AudioParameterFloat* gainParam;
};