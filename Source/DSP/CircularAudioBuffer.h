#pragma once
#include <JuceHeader.h>

class CircularAudioBuffer
{
public:
    CircularAudioBuffer() = default;
    ~CircularAudioBuffer() = default;

    void prepare(int numChannels, int capacitySamples);
    void pushBlock(const juce::AudioBuffer<float>& input);
    void getMostRecentWindow(juce::AudioBuffer<float>& out, int numSamples) const;

private:
    juce::AudioBuffer<float> buffer;
    int capacity = 0;
    int writePos = 0;
    int storedSamples = 0;
};
