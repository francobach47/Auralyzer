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
	castParameter(apvts, triggerLevelParamID, triggerLevelParam);
	castParameter(apvts, movingAverageParamID, movingAverageParam);
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
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		horizontalScaleParamID,
		"Hor Scale",
		[] {
			juce::StringArray labels;
			for (const auto& pair : horizontalScaleOptions)
				labels.add(pair.first);
			return labels;
		}(),
			6 // default = "10 ms"
			));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		verticalPositionParamID,
		"Ver Position",
		juce::NormalisableRange<float> {-4.0f, 4.0f, 0.01f },
		0.0f
	));

	juce::StringArray scaleChoices = { "A", "B", "C", "D" }; // valores placeholder
	layout.add(std::make_unique<juce::AudioParameterChoice>(
		verticalScaleParamID, "Ver Scale", scaleChoices, 0)); // default = tercera opción


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

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		triggerLevelParamID,
		"Trigger Level",
		juce::NormalisableRange<float>{ -4.0f, 4.0f, 0.01f },
		0.0f
	));

	layout.add(std::make_unique<juce::AudioParameterBool>(
		movingAverageParamID,
		"Moving Average",
		true
	));

	return layout;
}

void Parameters::update() noexcept
{
	horizontalScaleIndex = horizontalScaleParam->getIndex();
	horizontalPosition = horizontalPositionParam->get();
	
	verticalScaleIndex = verticalScaleParam->getIndex();
	verticalPosition = verticalPositionParam->get();

	modeValue = modeParam->getIndex();
	rangeValue = rangeParam->getIndex();

	triggerLevel = triggerLevelParam->get();

	plotMode = plotModeParam->getIndex();
}

float Parameters::getTriggerLevel() const noexcept
{
	return triggerLevelParam != nullptr ? triggerLevelParam->get() : triggerLevel;
}

float Parameters::getVerticalScaleInVolts() const
{
	if (rangeValue >= 0 && rangeValue < verticalScaleByRange.size() &&
		verticalScaleIndex >= 0 && verticalScaleIndex < 4)
		return verticalScaleByRange[rangeValue][verticalScaleIndex].second;

	return 1.0f; // valor seguro por default
}

float Parameters::getHorizontalScaleInSeconds() const
{
	if (horizontalScaleIndex >= 0 && horizontalScaleIndex < horizontalScaleOptions.size())
		return horizontalScaleOptions[horizontalScaleIndex].second;

	return 0.01f; // default: 10 ms/div
}
