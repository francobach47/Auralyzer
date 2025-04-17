#pragma once

#include <JuceHeader.h>
#include "DSP/FrequencyAnalyzer.h"
#include "DSP/Parameters.h"

class OscilloscopeAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    OscilloscopeAudioProcessor();
    ~OscilloscopeAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //juce::Point<int> getSavedSize() const;
    //void setSavedSize(const juce::Point<int>& size);

    void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq);

    bool checkForNewAnalyserData();

private:
    juce::AudioProcessorValueTreeState apvts{
        *this, nullptr, "Parameters", Parameters::createParameterLayout()
    };

    Parameters params;

    double sampleRate = 0;

    FrequencyAnalyzer<float> outputAnalyzer;

    //juce::Point<int> editorSize = { 900, 500 };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessor)
};
