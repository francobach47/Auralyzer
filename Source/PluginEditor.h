#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "DSP/Parameters.h"
#include "UI/RotaryKnob.h"
#include "UI/LookAndFeel.h"

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
    OscilloscopeAudioProcessor& audioProcessor;

    juce::GroupComponent verticalGroup, horizontalGroup;
    juce::GroupComponent optionsGroup;
    juce::GroupComponent plotGroup;

    RotaryKnob horizontalPositionKnob{ "Position", audioProcessor.apvts, horizontalPositionParamID, true };
    RotaryKnob horizontalScaleKnob{ "Scale", audioProcessor.apvts, horizontalScaleParamID, true };
    RotaryKnob verticalPositionKnob{ "Position", audioProcessor.apvts, verticalPositionParamID, true };
    RotaryKnob verticalScaleKnob{ "Scale", audioProcessor.apvts, verticalScaleParamID, true };
    RotaryKnob rangeKnob{ "Range", audioProcessor.apvts, rangeParamID, false };
    RotaryKnob modeKnob{ "Mode", audioProcessor.apvts, modeParamID, false };

    MainLookAndFeel mainLF;

    //float getFrequencyForPosition(float pos);
    //juce::Rectangle<int> plotFrame;

    //juce::Path frequencyResponse;
    //juce::Path analyzerPath;

    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessorEditor)
};
