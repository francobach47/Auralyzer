#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "../DSP/Trigger.h"
#include "LookAndFeel.h"

class TimeVisualizer : public juce::Component,
    public juce::Timer
{
public:
    TimeVisualizer(OscilloscopeAudioProcessor& processor);
    ~TimeVisualizer() override;

    void paint(juce::Graphics&) override;
    void timerCallback() override;

    // Visualization parameters
    void setVerticalGain(float gain) { verticalGain = gain; }
    void setVerticalOffset(float offset) { verticalOffset = offset; }
    void setHorizontalScale(float scale) { horizontalScale = scale; }
    void setHorizontalOffset(float offset) { horizontalOffset = offset; }
    
private:
    OscilloscopeAudioProcessor& processor;
    Trigger trigger;

    float verticalGain = 1.0f;
    float verticalOffset = 0.0f;

    float horizontalScale = 1.0f;
    float horizontalOffset = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TimeVisualizer)
};

