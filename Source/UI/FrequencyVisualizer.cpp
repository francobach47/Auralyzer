#include <JuceHeader.h>
#include "FrequencyVisualizer.h"
#include "LookAndFeel.h"

static float maxDB = 24.0f;
static float minDB = -40.0f;

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
    juce::MessageManagerLock mmLock;
    if (!mmLock.lockWasGained()) return;

	//juce::Graphics::ScopedSaveState state(g);

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

    for (int i = 0; i < 10; ++i)
    {
        g.setColour(juce::Colours::silver.withAlpha(0.3f));
        auto x = plotFrame.getX() + plotFrame.getWidth() * i * 0.1f;
        if (i > 0)
            g.drawVerticalLine(juce::roundToInt(x), float(plotFrame.getY()), float(plotFrame.getBottom()));

        g.setColour(juce::Colours::silver);
        auto freq = getFrequencyForPosition(i * 0.1f);
        g.drawFittedText((freq < 1000) ? juce::String(freq) + " Hz"
            : juce::String(freq / 1000, 1) + " kHz",
            juce::roundToInt(x + 3), plotFrame.getBottom() - 18, 50, 15, juce::Justification::left, 1);
    }

    g.setColour(juce::Colours::silver.withAlpha(0.3f));
    g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.25 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));
    g.drawHorizontalLine(juce::roundToInt(plotFrame.getY() + 0.75 * plotFrame.getHeight()), float(plotFrame.getX()), float(plotFrame.getRight()));

    g.setColour(juce::Colours::silver);
    g.drawFittedText(juce::String(maxDB) + " dB", plotFrame.getX() + 3, plotFrame.getY() + 2, 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(maxDB / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.25 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    g.drawFittedText(" 0 dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.5 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);
    g.drawFittedText(juce::String(-maxDB / 2) + " dB", plotFrame.getX() + 3, juce::roundToInt(plotFrame.getY() + 2 + 0.75 * plotFrame.getHeight()), 50, 14, juce::Justification::left, 1);

    g.reduceClipRegion(plotFrame);

    processor.createAnalyserPlot(analyzerPath, plotFrame, 20.0f);
    g.setColour(Colors::PlotSection::frequencyResponse);
    //g.drawFittedText("Output", plotFrame.reduced(8, 28), juce::Justification::topRight, 1);
    g.strokePath(analyzerPath, juce::PathStrokeType(2.0f));
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
    return 20.0f * std::pow(2.0f, pos * 10.0f);
}