#pragma once

#include <JuceHeader.h>

class Trigger
{
public:
	Trigger();

	void setParameters(float level, float offset, bool useFilter)
	{
		triggerLevel = level;
		triggerOffset = juce::jlimit(0.0f, 1.0f, offset);
		movingAverageEnabled = useFilter;
	}
	int findTriggerPoint(const juce::AudioBuffer<float>& buffer, int channel);
	void movingAverageFilter(const float* input, float* output);

private:
	juce::AudioBuffer<float> filteredBuffer;
	float         triggerLevel = 0.0f;
	float        triggerOffset = 0.0f;
	bool  movingAverageEnabled = false;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Trigger)
};