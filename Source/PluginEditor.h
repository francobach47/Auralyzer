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

    RotaryKnob horizontalScaleKnob{ "Scale", audioProcessor.apvts, horizontalScaleParamID, false };
    RotaryKnob horizontalPositionKnob{ "Position", audioProcessor.apvts, horizontalPositionParamID, true };
    RotaryKnob verticalScaleKnob{ "Scale", audioProcessor.apvts, verticalScaleParamID, false };
    RotaryKnob verticalPositionKnob{ "Position", audioProcessor.apvts, verticalPositionParamID, true };
    RotaryKnob modeKnob{ "Mode", audioProcessor.apvts, modeParamID, false };
    RotaryKnob rangeKnob{ "Range", audioProcessor.apvts, rangeParamID, false };

    MainLookAndFeel mainLF;

    juce::Rectangle<int> plotFrame;

    juce::Path frequencyResponse;
    juce::Path analyzerPath;

    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessorEditor)
};
