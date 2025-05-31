#include <JuceHeader.h>
#include "TimeVisualizer.h"

TimeVisualizer::TimeVisualizer(OscilloscopeAudioProcessor& p)
    : processor(p)
{
    setOpaque(true);
    startTimerHz(30);
    //updateTriggerParameters(0.0f, 0.5f, false);
}

TimeVisualizer::~TimeVisualizer()
{
}

void TimeVisualizer::setModeDC(bool enabled)
{
    if (modeDC != enabled)
    {
        modeDC = enabled;
        repaint(); 
    }
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

    const auto& buffer = processor.getAudioBuffer();
    const int numChannels = processor.getTotalNumInputChannels();
    const int numSamples = processor.getAudioBuffer().getNumSamples();

    int triggerSample = trigger.findTriggerPoint(buffer, 0);
    int displaySamples = juce::jlimit(16, numSamples - triggerSample, static_cast<int>(numSamples / horizontalScale));

    if (modeDC)
    {
        if (numSamples == 0) return;

        float minVal = std::numeric_limits<float>::max();
        float maxVal = std::numeric_limits<float>::lowest();

        for (int c = 0; c < numChannels; ++c)
        {
            const float* data = buffer.getReadPointer(c);
            for (int i = 0; i < numSamples; ++i)
            {
                minVal = std::min(minVal, data[i]);
                maxVal = std::max(maxVal, data[i]);
            }
        }

        float vpp = maxVal - minVal;

        float y = getHeight() / 2.0f - (vpp * verticalGain * getHeight() / 2.0f) - verticalOffset;


        g.setColour(juce::Colours::limegreen);
        g.drawLine(0.0f, y, (float)getWidth(), y, 2.0f);

        return; // saltea el dibujo de path convencional
    }

    juce::Path path;
    for (int i = 0; i < displaySamples; ++i)
    {
        int sampleIndex = triggerSample + i;
        if (sampleIndex >= numSamples) break;

        float sum = 0.0f;
        for (int c = 0; c < numChannels; ++c)
            sum += buffer.getSample(c, sampleIndex);

        float x = ((float)i / displaySamples + horizontalOffset) * getWidth();
        float y = getHeight() / 2.0f - (sum / numChannels * verticalGain * getHeight() / 2.0f) - verticalOffset;

        if (i == 0)
            path.startNewSubPath(x, y);
        else
            path.lineTo(x, y);
    }

    g.setColour(Colors::PlotSection::timeResponse);
    g.strokePath(path, juce::PathStrokeType(2.0f));

    float triggerY = getHeight() / 2.0f
        - (currentTriggerLevel * verticalGain * getHeight() / 2.0f)
        - verticalOffset;
    const float x0 = bounds.getX() + 6.0f;      //  margen
    const float size = 8.0f;                     // ancho del triang

    juce::Path tri;
    tri.addTriangle(x0 + size, triggerY,   
        x0, triggerY - size * 0.6f,   // base arriba
        x0, triggerY + size * 0.6f);  // base abajo


    g.setColour(Colors::PlotSection::triggerMarker); 
    g.fillPath(tri);
}


void TimeVisualizer::timerCallback()
{
    float triggerLevel = processor.getTriggerLevel();
    //float currentOffset = processor.triggerOffsetParam->get()
    //bool currentFilterEnabled = processor.filterEnabledParam->get() > 0.5f;

    updateTriggerParameters(triggerLevel, 0.0f, false); // change

    if (isVisible()) 
        repaint();
}

void TimeVisualizer::updateTriggerParameters(float level, float offset, bool filterEnabled)
{
    currentTriggerLevel = juce::jlimit(-1.0f, 1.0f, level);
    trigger.setParameters(level, offset, filterEnabled);
}