#pragma once

#include <JuceHeader.h>
#include "../DSP/Trigger.h"

class OscilloscopeAudioProcessor;

class OverviewDisplay : public juce::Component, private juce::Timer
{
public:
    OverviewDisplay(OscilloscopeAudioProcessor& processor);
    ~OverviewDisplay() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void updateViewport(float scaleSeconds, float offsetSeconds);

private:
    void timerCallback() override;
    void refreshWaveform();
    float getInterpolatedSample(const juce::AudioBuffer<float>& buffer, int channel, float index) const;
    float interpolateLinear(const juce::AudioBuffer<float>& buffer, int channel, float index) const;

    OscilloscopeAudioProcessor& processor;
    juce::Image waveformImage;

    float currentScale = 0.1f;
    float currentOffset = 0.0f;
    float totalDuration = 10.0f; // duración total mostrada en mini display

    Trigger trigger;
};