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
    void copyFullBuffer(juce::AudioBuffer<float>& dest) const; 
    float computeLastVpp();

    int getSize() const { return buffer.getNumSamples(); }
    int getStoredSamples() const { return storedSamples; }
    int getNumChannels() const { return buffer.getNumChannels(); }

private:
    juce::AudioBuffer<float> buffer;
    int capacity = 0;
    int writePos = 0;        
    int storedSamples = 0;
};
