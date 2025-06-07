#pragma once

#include <JuceHeader.h>
#include "DSP/Parameters.h"
#include "DSP/FFT.h"
#include "Serial/SerialDevice.h"
#include "DSP/Trigger.h"
#include "DSP/CircularAudioBuffer.h"

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

    // Serial Communication
    SerialDevice& getSerialDevice() { return serialDevice; }
    const SerialDevice& getSerialDevice() const { return serialDevice; }

    Parameters params;

    float getTriggerLevel() const { return params.getTriggerLevel(); }

    //Circular Buffer para representacion temporal
    CircularAudioBuffer& getCircularBuffer() { return circularBuffer; }

    //CalibrationLevel
    void startLevelCalibration();
    float getCalibrationFactor() const;
    void setCalibrationFactorAC(float factor) { calibrationFactorAC = factor; }
    void setCalibrationFactorDC(float factor) { calibrationFactorDC = factor; }

    float getCorrectedVoltage(float vppMedido) const;

    void setSineEnabled(bool enabled);

private:
    juce::AudioBuffer<float> audioTimeBuffer;
    CircularAudioBuffer circularBuffer;

    FFT frequencyAnalyzer;

    SerialDevice serialDevice;
    bool lastFrequencyModeState = false;

    bool isCalibratingLevel = false;
    float calibrationFactorAC = 1.0f;
    int   calibrationRangeAC = 2;

    float calibrationFactorDC = 1.0f;
    int   calibrationRangeDC = 2;

    int calibrationRange = 2; // por defecto calibrado en el rango 1 V – 10 V

    // Generador de seno para calibración
    double phase = 0.0;
    double phaseIncrement = 0.0;
    bool sineEnabled = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OscilloscopeAudioProcessor)
};
