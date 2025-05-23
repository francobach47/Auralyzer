#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "DSP/Parameters.h"

#include "UI/TimeVisualizer.h"
#include "UI/FrequencyVisualizer.h"
#include "UI/RotaryKnob.h"
#include "UI/LookAndFeel.h"

class OscilloscopeAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::AudioProcessorValueTreeState::Listener
{
public:
    OscilloscopeAudioProcessorEditor(OscilloscopeAudioProcessor&);
    ~OscilloscopeAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    OscilloscopeAudioProcessor& audioProcessor;

    TimeVisualizer timeVisualizer;
    FrequencyVisualizer frequencyVisualizer;

    juce::GroupComponent verticalGroup, horizontalGroup;
    juce::GroupComponent optionsGroup;
    juce::GroupComponent plotGroup;

    RotaryKnob horizontalPositionKnob{ "Position", audioProcessor.apvts, horizontalPositionParamID, true };
    RotaryKnob horizontalScaleKnob{ "Scale", audioProcessor.apvts, horizontalScaleParamID, false };
    RotaryKnob verticalPositionKnob{ "Position", audioProcessor.apvts, verticalPositionParamID, true };
    RotaryKnob verticalScaleKnob{ "Scale", audioProcessor.apvts, verticalScaleParamID, true };
    RotaryKnob rangeKnob{ "Range", audioProcessor.apvts, rangeParamID, false };
    RotaryKnob modeKnob{ "Mode", audioProcessor.apvts, modeParamID, false };
    //RotaryKnob triggerLevelKnob{ "Trig Level", audioProcessor.apvts, triggerLevelParamID, true };

    MainLookAndFeel mainLF;

    juce::TextButton plotModeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> plotModeAttachment;
    bool isFrequencyMode;

    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeAudioProcessorEditor)
};