#include <JuceHeader.h>
#include "TimeVisualizer.h"
#include "LookAndFeel.h"

TimeVisualizer::TimeVisualizer(OscilloscopeAudioProcessor& p)
    : processor(p)
{
    setOpaque(true);
    startTimerHz(30);
}

TimeVisualizer::~TimeVisualizer()
{
}

void TimeVisualizer::paint(juce::Graphics& g)
{
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
    g.drawRoundedRectangle(bounds, cornerRadius, borderThickness);

    const int numChannels = processor.getTotalNumInputChannels();
    const int numSamples = processor.getAudioBuffer().getNumSamples();
    const float* input = processor.getAudioBuffer().getReadPointer(0); // canal 0

    // --- Filtro pasa bajos simple
    std::vector<float> filtered(numSamples, 0.0f);
    filtered[0] = input[0];
    for (int i = 1; i < numSamples - 1; ++i)
        filtered[i] = (input[i - 1] + input[i] + input[i + 1]) / 3.0f;

    // --- Buscar trigger a partir de offset
    int triggerStart = juce::jlimit(1, numSamples - 2, int(triggerOffset * numSamples));
    int triggerSample = triggerStart;
    for (int i = triggerStart + 1; i < numSamples; ++i)
    {
        if (filtered[i - 1] < triggerLevel && filtered[i] >= triggerLevel)
        {
            triggerSample = i;
            break;
        }
    }

    // --- Determinar cuántas muestras mostrar según el zoom (horizontalScale)
    int displaySamples = juce::jlimit(16, numSamples - triggerSample, int(numSamples / horizontalScale));

    juce::Path path;
    for (int i = 0; i < displaySamples; ++i)
    {
        int sampleIndex = triggerSample + i;
        if (sampleIndex >= numSamples) break;

        float sum = 0.0f;
        for (int c = 0; c < numChannels; ++c)
            sum += processor.getAudioBuffer().getSample(c, sampleIndex);

        float x = ((float)i / displaySamples + horizontalOffset) * getWidth(); // offset en coordenadas
        float y = getHeight() / 2.0f - (sum / numChannels * verticalGain * getHeight() / 2.0f) - verticalOffset;

        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(Colors::PlotSection::timeResponse);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}


void TimeVisualizer::timerCallback()
{
    if (isVisible())
        repaint();
}