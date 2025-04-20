#pragma once

#include <JuceHeader.h>
#include "DSP/Parameters.h"
#include "DSP/FFT.h"

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

    // Frequency Visualizer
    void createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq);
    bool checkForNewAnalyserData();

    // Timer Visualizer
    juce::AudioBuffer<float>& getAudioBuffer() { return audioTimeBuffer; }
    int getNumInputChannels() const { return audioTimeBuffer.getNumChannels(); }

    // APVTS
    juce::AudioProcessorValueTreeState apvts{
        *this, nullptr, "Parameters", Parameters::createParameterLayout()
    };

    Parameters params;

private:

    juce::AudioBuffer<float> audioTimeBuffer;

    FFT frequencyAnalyzer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessor)
};
