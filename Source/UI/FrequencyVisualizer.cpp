#include <JuceHeader.h>
#include "FrequencyVisualizer.h"
#include "LookAndFeel.h"

static float maxDB = 24.0f;
static float minDB = -80.0f;

FrequencyVisualizer::FrequencyVisualizer(OscilloscopeAudioProcessor& p)
    : processor(p)
{
    setOpaque(true);
    startTimerHz(60);
}

FrequencyVisualizer::~FrequencyVisualizer()
{
}

void FrequencyVisualizer::paint (juce::Graphics& g)
{
    const bool bypassed = processor.apvts.getRawParameterValue(bypassParamID.getParamID())->load();

    juce::MessageManagerLock mmLock;
    if (!mmLock.lockWasGained()) return;

    const float cornerRadius = 8.0f;
    const float borderThickness = 4.0f;
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Colors::PlotSection::background);
    g.fillRoundedRectangle(bounds, cornerRadius);

    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);
    g.reduceClipRegion(clipPath);

    g.setColour(Colors::PlotSection::outline);
    g.drawRoundedRectangle(
        getLocalBounds().toFloat(),
        cornerRadius,
        borderThickness
    );

	g.setFont(12.0f);
	g.setColour(juce::Colours::silver);
	g.drawRoundedRectangle(plotFrame.toFloat(), 5, 2);

    // ===============================
    // FREQUENCY GRID (X AXIS [Hz])
    // ===============================
    std::vector<float> logFrequencies = {
        31.5f, 63.0f, 125.0f, 250.0f,
        500.0f, 1000.0f, 2000.0f,
        4000.0f, 8000.0f, 16000.0f
    };

    for (auto freq : logFrequencies)
    {
        float normX = getPositionForFrequency(freq);
        float x = plotFrame.getX() + normX * plotFrame.getWidth();

        g.setColour(juce::Colours::silver.withAlpha(0.1f));
        g.drawVerticalLine(juce::roundToInt(x), float(plotFrame.getY()), float(plotFrame.getBottom()));

        // Label
        g.setColour(juce::Colours::silver.withAlpha(0.8f));
        juce::String label = (freq >= 1000.0f)
            ? juce::String(freq / 1000.0f, 1) + " kHz"
            : juce::String((int)freq) + " Hz";

        g.drawFittedText(label, juce::roundToInt(x + 3), plotFrame.getBottom() - 18, 50, 15, juce::Justification::left, 1);
    }

    // ===============================
    // LEVEL GRID ( Y AXIS [dB])
    // ===============================
    const float dBStep = 8.0f;
    const int numSteps = static_cast<int>((maxDB - minDB) / dBStep);

    for (int i = 0; i <= numSteps; ++i)
    {
        float dBValue = maxDB - i * dBStep;
        float dBZoomPadding = 0.05f; 
        float normY = dBZoomPadding + (1.0f - 2.0f * dBZoomPadding) * ((maxDB - dBValue) / (maxDB - minDB));
        float y = plotFrame.getY() + normY * plotFrame.getHeight();

        g.setColour(juce::Colours::silver.withAlpha(0.1f));
        g.drawHorizontalLine(juce::roundToInt(y), float(plotFrame.getX()), float(plotFrame.getRight()));

        // Label
        g.setColour(juce::Colours::silver.withAlpha(0.8f));
        g.drawFittedText(juce::String((int)dBValue) + " dB", plotFrame.getX() + 3, juce::roundToInt(y - 7), 50, 14, juce::Justification::left, 1);
    }

    if (!bypassed)
    {
        processor.createAnalyserPlot(analyzerPath, plotFrame, minDB, maxDB);
        g.setColour(Colors::PlotSection::frequencyResponse);
        g.strokePath(analyzerPath, juce::PathStrokeType(2.0f));
       
        auto harmonics = processor.getHarmonicLabels();

        for (const auto& [freq, dB] : harmonics)
        {
            float normX = getPositionForFrequency(freq);
            float x = plotFrame.getX() + normX * plotFrame.getWidth();
            float normY = juce::jmap(dB, minDB, maxDB, 1.0f, 0.0f);
            float y = plotFrame.getY() + normY * plotFrame.getHeight();

            juce::String label;
            if (freq >= 1000.0f)
                label = juce::String(freq / 1000.0f, 1) + "k\n" + juce::String(dB, 1) + " dB";
            else
                label = juce::String((int)freq) + "\n" + juce::String(dB, 1) + " dB";

            g.setColour(juce::Colours::yellow);
            g.setFont(11.0f);
            g.drawFittedText(label, juce::roundToInt(x) - 20, juce::roundToInt(y) - 25, 40, 25, juce::Justification::centred, 2);
        }

    }
}

void FrequencyVisualizer::resized()
{
    plotFrame = getLocalBounds();
}

void FrequencyVisualizer::timerCallback()
{
    if (processor.checkForNewAnalyserData())
    {
        repaint(plotFrame);
    }
}

float FrequencyVisualizer::getFrequencyForPosition(float pos)
{
    return 20.0f * std::pow(20000.0f / 20.0f, pos); //  log scale
}

float FrequencyVisualizer::getPositionForFrequency(float freq)
{
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;

    // "Zoom out" horizontally 0.05 to 0.95 of the width
    float norm = std::log(freq / minFreq) / std::log(maxFreq / minFreq);
    return 0.05f + 0.90f * norm; // 0.90 full width, centered
}