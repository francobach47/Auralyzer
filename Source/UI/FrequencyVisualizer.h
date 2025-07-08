#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class FrequencyVisualizer  : public juce::Component,
                             public juce::Timer
{
public:
    FrequencyVisualizer(OscilloscopeAudioProcessor& processor);
    ~FrequencyVisualizer() override;

    void resized() override;
    void paint (juce::Graphics&) override;

    void timerCallback() override;

private:
    float getFrequencyForPosition(float pos);
    float getPositionForFrequency(float freq);

    OscilloscopeAudioProcessor& processor;

    juce::Rectangle<int> plotFrame;

    juce::Path frequencyResponse;
    juce::Path analyzerPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyVisualizer)
};
