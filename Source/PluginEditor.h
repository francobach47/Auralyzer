#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class OscilloscopeAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                          public juce::Timer
{
public:
    OscilloscopeAudioProcessorEditor (OscilloscopeAudioProcessor&);
    ~OscilloscopeAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override; // juce::Timer virtual void

private:
    float getFrequencyForPosition(float pos);

    OscilloscopeAudioProcessor& audioProcessor;

    juce::Rectangle<int> plotFrame;

    juce::Path frequencyResponse;
    juce::Path analyzerPath;

    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessorEditor)
};
