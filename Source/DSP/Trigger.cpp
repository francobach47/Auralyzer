#include "Trigger.h"

Trigger::Trigger()
{
    filteredBuffer.clear();
}

int Trigger::findTriggerPoint(const juce::AudioBuffer<float>& buffer, int channel)
{
	const int numSamples = buffer.getNumSamples();
	const float* input = buffer.getReadPointer(channel);

    //if (filterEnabled.load())
    //{
    //    if (filteredBuffer.size() < static_cast<size_t>(numSamples))
    //    {
    //        filteredBuffer.resize(numSamples);
    //    }
    //    applyMovingAverageFilter(input, filteredBuffer.data(), numSamples);
    //}

    // TODO: See Channel
    const float* data   = movingAverageEnabled ? filteredBuffer.getReadPointer(0) : input; 
    int triggerStart = juce::jlimit(1, numSamples - 2, static_cast<int>(triggerOffset * numSamples));

    // Find ascending zero crossing point
    for (int i = triggerStart; i < numSamples - 1; ++i)
    {
        if (data[i - 1] < triggerLevel && data[i] >= triggerLevel)
        {
            return i;
        }
    }

    return triggerStart;
}


void Trigger::movingAverageFilter(const float* input, float* output)
{

}