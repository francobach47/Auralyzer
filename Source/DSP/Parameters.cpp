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
	castParameter(apvts, horizontalPositionParamID, horizontalPositionParam);
	castParameter(apvts, horizontalScaleParamID, horizontalScaleParam);
	castParameter(apvts, verticalPositionParamID, verticalPositionParam);
	castParameter(apvts, verticalScaleParamID, verticalScaleParam);
	castParameter(apvts, rangeParamID, rangeParam);
	castParameter(apvts, modeParamID, modeParam);
	castParameter(apvts, plotModeParamID, plotModeParam);
	//castParameter(apvts, triggerLevelParamID, triggerLevelParam);
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		horizontalPositionParamID,
		"Hor Position",
		juce::NormalisableRange<float> {-5.0f, 5.0f, 0.01f },
		0.0f
	));
	layout.add(std::make_unique<juce::AudioParameterInt>(
		horizontalScaleParamID,
		"Hor Scale",
		1, 8, 0
	));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		verticalPositionParamID,
		"Ver Position",
		juce::NormalisableRange<float> {-5.0f, 5.0f, 0.01f },
		0.0f
	));
	layout.add(std::make_unique<juce::AudioParameterInt>(
		verticalScaleParamID,
		"Ver Scale",
		-5, 5, 0
	));

	juce::StringArray rangeOptions = {
		"10 mV - 100 mV",
		"0.1 V - 1 V",
		"1 V - 10 V",
		"10 V - 100 V"
	};
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		rangeParamID, "Range", rangeOptions, 0));

	juce::StringArray modeOptions = {
		"AC",
		"DC",
		"Calibration"
	};
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		modeParamID, "Mode", modeOptions, 0));

	juce::StringArray plotOptions = {
		"Time",
		"Frequency"
	};
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		plotModeParamID,
		"Plot Option",
		plotOptions,
		0
	));

	/*layout.add(std::make_unique<juce::AudioParameterFloat>(
		triggerLevelParamID,
		"Trigger Level",
		juce::NormalisableRange<float>{ -1.0f, 1.0f, 0.01f },
		0.0f
	));*/

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

	//triggerLevel = triggerLevelParam->get();

	plotMode = plotModeParam->getIndex();
}