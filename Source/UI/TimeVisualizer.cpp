#include <JuceHeader.h>
#include "TimeVisualizer.h"

//==============================================================================
TimeVisualizer::TimeVisualizer(OscilloscopeAudioProcessor& p)
    : processor(p)
{
    setOpaque(true);
    startTimerHz(30);
}

TimeVisualizer::~TimeVisualizer()
{
}

void TimeVisualizer::paint (juce::Graphics& g)
{
    juce::MessageManagerLock mmLock;

    if (!mmLock.lockWasGained())
        return;

    g.fillAll(juce::Colours::transparentBlack);

    if (auto* playHead = processor.getPlayHead())
    {
        juce::AudioPlayHead::CurrentPositionInfo pos;
        playHead->getCurrentPosition(pos);
    }

    const int numChannels = processor.getTotalNumInputChannels();
    const int numSamples = processor.getAudioBuffer().getNumSamples();

    juce::Path path;
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float sum = 0.0f;

        for (int channel = 0; channel < numChannels; ++channel)
        {
            const float* channelData = processor.getAudioBuffer().getReadPointer(channel);
            sum += channelData[sample];
        }

        float x = static_cast<float>(sample) / numSamples * getWidth();
        float y = getHeight() / 2 + sum / numChannels * getHeight() / 2;

        if (sample == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(juce::Colours::red);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void TimeVisualizer::timerCallback()
{
    if (isVisible())
        repaint();
}
