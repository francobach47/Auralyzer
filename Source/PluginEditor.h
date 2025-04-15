#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DSP/Parameters.h"
#include "UI/RotaryKnob.h"
#include "UI/LookAndFeel.h"

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

    juce::GroupComponent verticalGroup, horizontalGroup;

    //RotaryKnob gainKnob{ "Gain", audioProcessor.apvts, gainParamID, true };

    RotaryKnob horizontalScaleKnob{ "Scale", false}; // add 2 parameters
    RotaryKnob horizontalPositionKnob{ "Position", true }; // add 2 parameters
    //RotaryKnob verticalScaleKnob{ "Scale" }; // add 2 parameters
    //RotaryKnob verticalPositionKnob{ "Position" }; // add 2 parameters
    //RotaryKnob modeKnob{ "Mode" }; // add 2 parameters
    //RotaryKnob rangeKnob{ "Range" }; // add 2 parameters

    juce::Rectangle<int> plotFrame;

    juce::Path frequencyResponse;
    juce::Path analyzerPath;

    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessorEditor)
};
