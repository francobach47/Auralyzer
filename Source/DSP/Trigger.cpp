#include "Trigger.h"

Trigger::Trigger()
{
    filteredBuffer.clear();
}

int Trigger::findTriggerPoint(const juce::AudioBuffer<float>& buffer, int channel)
{
    const int numSamples = buffer.getNumSamples();
    const float* input = buffer.getReadPointer(channel);

    if (movingAverageEnabled)
    {
        if (filteredBuffer.getNumSamples() != numSamples)
            filteredBuffer.setSize(1, numSamples, false, false, true);

        float* filteredData = filteredBuffer.getWritePointer(0);
        movingAverageFilter(input, filteredData, numSamples);
    }

    const float* data = movingAverageEnabled ? filteredBuffer.getReadPointer(0) : input;
    int triggerStart = juce::jlimit(1, numSamples - 2, static_cast<int>(triggerOffset * numSamples));

    for (int i = triggerStart; i < numSamples - 1; ++i)
    {
        if (data[i - 1] < triggerLevel && data[i] >= triggerLevel)
        {
            return i;
        }
    }

    return triggerStart;
}

void Trigger::movingAverageFilter(const float* input, float* output, int numSamples)
{
    const int windowSize = 5; // Ventana de 5 muestras (ajustable)
    float sum = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        int count = 0;
        sum = 0.0f;

        for (int j = i - windowSize / 2; j <= i + windowSize / 2; ++j)
        {
            if (j >= 0 && j < numSamples)
            {
                sum += input[j];
                ++count;
            }
        }

        output[i] = sum / static_cast<float>(count);
    }
}
