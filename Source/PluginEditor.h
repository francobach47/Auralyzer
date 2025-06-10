#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#include "DSP/Parameters.h"

#include "UI/TimeVisualizer.h"
#include "UI/FrequencyVisualizer.h"
#include "UI/RotaryKnob.h"
#include "UI/LookAndFeel.h"

class OscilloscopeAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::AudioProcessorValueTreeState::Listener,
    private juce::Timer
{
public:
    OscilloscopeAudioProcessorEditor(OscilloscopeAudioProcessor&);
    ~OscilloscopeAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    void actualizarKnobsDesdeESP(uint8_t modo, uint8_t rango);
    void bloquearControles(bool pluginControls);

private:
    OscilloscopeAudioProcessor& audioProcessor;

    TimeVisualizer timeVisualizer;
    FrequencyVisualizer frequencyVisualizer;
    juce::TextButton probesCalibrationButton{ "Probes" };
    bool isCalibrating = false;

    juce::TextButton levelCalibrationButton{ "Calibrator" };
    bool isLevelCalibrating = false;

    juce::GroupComponent verticalGroup, horizontalGroup;
    juce::GroupComponent optionsGroup;
    juce::GroupComponent plotGroup;
    juce::GroupComponent triggerGroup;
    juce::GroupComponent calibrationGroup; 
    juce::GroupComponent screenShotGroup;

    RotaryKnob horizontalPositionKnob{ "Position", audioProcessor.apvts, horizontalPositionParamID, true };
    RotaryKnob horizontalScaleKnob{ "Scale", audioProcessor.apvts, horizontalScaleParamID, false };
    RotaryKnob verticalPositionKnob{ "Position", audioProcessor.apvts, verticalPositionParamID, true };
    RotaryKnob verticalScaleKnob{ "Scale", audioProcessor.apvts, verticalScaleParamID, false };
    RotaryKnob rangeKnob{ "Range", audioProcessor.apvts, rangeParamID, false };
    RotaryKnob modeKnob{ "Mode", audioProcessor.apvts, modeParamID, false };
    RotaryKnob triggerLevelKnob{ "Level", audioProcessor.apvts, triggerLevelParamID, true };

    MainLookAndFeel mainLF;

    juce::TextButton plotModeButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> plotModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modeAttachment, rangeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> movingAverageAttachment;

    bool isFrequencyMode;
    bool pluginIsInControl = true;

    void timerCallback() override;
    juce::ComboBox serialPortSelector;
    juce::Label serialPortLabel;

    juce::TextButton movingAverageButton;

    juce::SharedResourcePointer<juce::TooltipWindow> tooltipWindow;

    juce::TextButton sineButton;

    juce::ImageButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;

    juce::TextButton snapshotButton{ "Print" };
    juce::TextButton clearSnapshotsButton{ "Clear" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeAudioProcessorEditor)
};