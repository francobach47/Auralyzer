#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"

class TimeVisualizer  : public juce::Component, 
                         public juce::Timer
{
public:
    TimeVisualizer(OscilloscopeAudioProcessor& processor);
    ~TimeVisualizer() override;

    void paint (juce::Graphics&) override;

    void timerCallback() override;
private:
    OscilloscopeAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimeVisualizer)
};
