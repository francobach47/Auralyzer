#include "Parameters.h"

template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts,
	const juce::ParameterID& id, T& destination)
{
	destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
	jassert(destination); // parameter does not exist or wrong type
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
	castParameter(apvts, horizontalScaleParamID, horizontalScaleParam);
	castParameter(apvts, horizontalPositionParamID, horizontalScaleParam);
	castParameter(apvts, verticalScaleParamID, verticalScaleParam);
	castParameter(apvts, verticalPositionParamID, verticalPositionParam);
	castParameter(apvts, rangeParamID, rangeParam);
	castParameter(apvts, modeParamID, modeParam);
	castParameter(apvts, timeFreqParamID, timeFreqParam);
}


juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterInt>(
		horizontalScaleParamID,
		"Horizontal Scale",
		-5, 5, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>(
		horizontalPositionParamID,
		"Horizontal Position",
		juce::NormalisableRange<float> {-5.0f, 5.0f},
		0.0f
	));

	layout.add(std::make_unique<juce::AudioParameterInt>(
		verticalScaleParamID,
		"Vertical Scale",
		-5, 5, 0));
	layout.add(std::make_unique<juce::AudioParameterFloat>(
		verticalPositionParamID,
		"Vertical Position",
		juce::NormalisableRange<float> {-5.0f, 5.0f, 0.5f},
		0.0f
	));

	juce::StringArray modeOptions = {
		"AC",
		"DC",
		"Calibration"
	};
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		modeParamID, "Mode", modeOptions, 0));

	juce::StringArray rangeOptions = {
		"10 mV - 100 mV",
		"0.1 V - 1 V",
		"1 V - 10 V",
		"10 V - 100 V"
	};
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		rangeParamID, "Range", rangeOptions, 0));

	layout.add(std::make_unique<juce::AudioParameterBool>(
		timeFreqParamID, "Frequency / Time", false));

	return layout;
}

void Parameters::update() noexcept
{
	horizontalScale = horizontalScaleParam->get();
	horizontalPosition = horizontalPositionParam->get();
	
	verticalScale = verticalScaleParam->get();
	verticalPosition = verticalPositionParam->get();

	modeValue = modeParam->getIndex();
	rangeValue = rangeParam->getIndex();

	freqTyme = timeFreqParam->get();
}