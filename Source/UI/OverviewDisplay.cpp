#include "OverviewDisplay.h"
#include "../PluginProcessor.h"
#include "TimeVisualizer.h"

OverviewDisplay::OverviewDisplay(OscilloscopeAudioProcessor& p)
    : processor(p)
{
    waveformImage = juce::Image(juce::Image::RGB, 512, 80, true);
    startTimerHz(30);
}

OverviewDisplay::~OverviewDisplay() {}

void OverviewDisplay::updateViewport(float scaleSeconds, float offsetSeconds)
{
    currentScale = scaleSeconds;
    currentOffset = offsetSeconds;
}

void OverviewDisplay::resized()
{
    waveformImage = juce::Image(juce::Image::RGB, getWidth(), getHeight(), true);
}

void OverviewDisplay::timerCallback()
{
    refreshWaveform();
    repaint();
}

void OverviewDisplay::refreshWaveform()
{
    const bool bypass = processor.apvts.getRawParameterValue(bypassParamID.getParamID())->load() > 0.5f;
    if (!bypass)
    {
        auto& circular = processor.getCircularBuffer();
        const int numChannels = circular.getNumChannels();
        const int numSamples = circular.getStoredSamples();

        if (numChannels == 0 || numSamples < 16)
            return;

        juce::AudioBuffer<float> temp;
        circular.getMostRecentWindow(temp, numSamples);
        if (temp.getNumSamples() < 16) return;

        const float sampleRate = processor.getSampleRate();
        const float secondsPerDiv = processor.params.getHorizontalScaleInSeconds();

        const float mainVisibleTime = secondsPerDiv * 10.0f; // 10 divisiones en display principal
        const float overviewVisibleTime = mainVisibleTime * 5.0f; // 5x zoom out en overview

        int totalSamplesNeeded = static_cast<int>(overviewVisibleTime * sampleRate) + 2048;
        juce::AudioBuffer<float> buffer;
        circular.getMostRecentWindow(buffer, totalSamplesNeeded);
        if (buffer.getNumSamples() < 16) return;

        const int numSamplesToPlot = buffer.getNumSamples();
        const int numChannelsToPlot = buffer.getNumChannels();
        const int w = getWidth();
        const int h = getHeight();

        juce::Graphics g(waveformImage);
        g.fillAll(juce::Colours::black);

        // === Trigger + Offset ===
        float triggerSample = trigger.findTriggerPoint(buffer, 0);
        float offsetSamples = -processor.params.horizontalPosition * secondsPerDiv * sampleRate;
        float startSampleIndex = triggerSample + offsetSamples;

        float pixelsPerSecond = (float)w / overviewVisibleTime;

        // === Escalado vertical fijo (90% altura, centrado) ===
        float yCenter = h / 2.0f;
        float yScale = h * 1.8f;

        juce::Path path;

        for (int i = 0; i < w; ++i)
        {
            float t = ((float)i / (float)w) * overviewVisibleTime;
            float sampleIndex = startSampleIndex + t * sampleRate;

            while (sampleIndex >= numSamplesToPlot) sampleIndex -= numSamplesToPlot;
            while (sampleIndex < 0) sampleIndex += numSamplesToPlot;

            float sum = 0.0f;
            for (int c = 0; c < numChannelsToPlot; ++c)
                sum += getInterpolatedSample(buffer, c, sampleIndex);

            float value = sum / (float)numChannelsToPlot;
            float y = yCenter - value * yScale;

            if (i == 0) path.startNewSubPath((float)i, y);
            else        path.lineTo((float)i, y);
        }

        g.setColour(juce::Colours::lime);
        g.strokePath(path, juce::PathStrokeType(1.2f));

        // === Viewport que representa las 10 divisiones ===
        float viewportWidth = mainVisibleTime * pixelsPerSecond;

        // Mismo cálculo que en TimeVisualizer pero aplicado a overview
        float normOffset = processor.params.horizontalPosition;
        float centerX = (float)w * 0.5f - normOffset * (mainVisibleTime / overviewVisibleTime) * (float)w * 0.5f;

        float viewportStart = centerX - viewportWidth * 0.5f;
        viewportStart = juce::jlimit(0.0f, (float)(w - viewportWidth), viewportStart);

        g.setColour(juce::Colours::white.withAlpha(0.4f));
        g.fillRect(viewportStart, 0.0f, viewportWidth, (float)h);

        g.setColour(juce::Colours::white);
        g.drawRect(viewportStart, 0.0f, viewportWidth, (float)h, 1.5f);
    }
}

void OverviewDisplay::paint(juce::Graphics& g)
{
    const bool bypass = processor.apvts.getRawParameterValue(bypassParamID.getParamID())->load() > 0.5f;
    const float cornerRadius = 8.0f;
    const float borderThickness = 4.0f;
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Colors::PlotSection::background);
    g.fillRoundedRectangle(bounds, cornerRadius);

    if (!bypass)
    {

        juce::Path clipPath;
        clipPath.addRoundedRectangle(bounds, cornerRadius);
        g.reduceClipRegion(clipPath);

        g.drawImage(waveformImage, bounds);

        g.setColour(Colors::PlotSection::outline);
        g.drawRoundedRectangle(bounds, cornerRadius, borderThickness);
    }

}

float OverviewDisplay::getInterpolatedSample(const juce::AudioBuffer<float>& buffer, int channel, float index) const
{
    return interpolateLinear(buffer, channel, index);
}

float OverviewDisplay::interpolateLinear(const juce::AudioBuffer<float>& buffer, int channel, float index) const
{
    int i0 = (int)std::floor(index);
    int i1 = juce::jmin(i0 + 1, buffer.getNumSamples() - 1);
    float frac = index - (float)i0;
    return (1.0f - frac) * buffer.getSample(channel, i0) + frac * buffer.getSample(channel, i1);
}