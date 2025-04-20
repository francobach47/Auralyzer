#include <JuceHeader.h>
#include "FrequencyVisualizer.h"
#include "LookAndFeel.h"

static float maxDB = 24.0f;

FrequencyVisualizer::FrequencyVisualizer(OscilloscopeAudioProcessor& p)
    : audioProcessor(p) 
{
    startTimerHz(30);
}

FrequencyVisualizer::~FrequencyVisualizer()
{
}

void FrequencyVisualizer::paint (juce::Graphics& g)
{
    const auto frequencyResponseColor = juce::Colours::greenyellow;

	juce::Graphics::ScopedSaveState state(g);

	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

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

    g.setFont(16.0f);
    //audioProcessor.createAnalyserPlot(analyzerPath, plotFrame, 20.0f);
    g.setColour(frequencyResponseColor);
    g.drawFittedText("Output", plotFrame.reduced(8, 28), juce::Justification::topRight, 1);
    g.strokePath(analyzerPath, juce::PathStrokeType(1.0f));

}

void FrequencyVisualizer::timerCallback()
{
    //if (audioProcessor.checkForNewAnalyserData())
    //{
    //    repaint(plotFrame);
    //}
}

float FrequencyVisualizer::getFrequencyForPosition(float pos)
{
    return 20.0f * std::pow(2.0f, pos * 10.0f);
}