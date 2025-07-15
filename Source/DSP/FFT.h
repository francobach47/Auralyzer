#pragma once

#include <JuceHeader.h>

class FFT : public juce::Thread
{
public:
	FFT();
	~FFT() override;

    void addAudioData(const juce::AudioBuffer<float>& buffer, int startChannel, int numChannels);
    void setUpFrequencyAnalyzer(int audioFifoSize, float sampleRateToUse);
    bool checkForNewData();
    void createPath(juce::Path& p, const juce::Rectangle<float> bounds, float minFreq, float dBMin, float dBMax);
    std::vector<std::pair<float, float>> getHarmonicsInDB(int maxHarmonics = 5, float minDB = -80.0f) const;

private:
    void run() override;
    float indexToX(float index, float minFreq) const;
    float binToY(float bin, const juce::Rectangle<float> bounds, float dBMin, float dBMax) const;

    juce::WaitableEvent waitForData;
    juce::CriticalSection pathCreationLock;

    double sampleRate;

    juce::dsp::FFT fft{ 12 };
    juce::dsp::WindowingFunction<float> windowing{ 1 << 12, juce::dsp::WindowingFunction<float>::hann, true };
    juce::AudioBuffer<float> fftBuffer{ 1, (1 << 12) * 2 };
    juce::AudioBuffer<float> averager{ 9, (1 << 12) / 2 };
    int averagerPtr = 1;
    juce::AbstractFifo abstractFifo{ 48000 };
    juce::AudioBuffer<float> audioFifo{ 1, 48000 };
    std::atomic<bool> newDataAvailable{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FFT)
};