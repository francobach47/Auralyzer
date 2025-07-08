#pragma once

#include <JuceHeader.h>

class SignalAnalysis
{
public:
    static float computeRMS(const juce::AudioBuffer<float>& buffer, float calibrationFactor);
    static float computeVpp(float minY, float maxY, float pixelsPerDiv, float voltsPerDiv);
    static float computeFrequency(const juce::AudioBuffer<float>& buffer, float sampleRate);
    static float computeTHD(const juce::AudioBuffer<float>& buffer, float sampleRate, int fftOrder = 10, int maxHarmonics = 5);

};
