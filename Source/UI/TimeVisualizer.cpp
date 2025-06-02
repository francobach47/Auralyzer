#include <JuceHeader.h>
#include "TimeVisualizer.h"

TimeVisualizer::TimeVisualizer(OscilloscopeAudioProcessor& p)
    : processor(p)
{
    setOpaque(true);
    startTimerHz(30);
}

TimeVisualizer::~TimeVisualizer() {}

void TimeVisualizer::setModeDC(bool enabled)
{
    if (modeDC != enabled)
    {
        modeDC = enabled;
        repaint();
    }
}

void TimeVisualizer::setVerticalOffsetInDivisions(float offsetDivs)
{
    // Converts de divisiones a pxeles usando 8 divisiones verticales
    const float pixelsPerDivision = getHeight() / 8.0f;
    verticalOffset = offsetDivs * pixelsPerDivision;
    repaint();
}

void TimeVisualizer::drawGrid(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const int numHDivs = 10;
    const int numVDivs = 8;
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();
    const float hStep = w / numHDivs;
    const float vStep = h / numVDivs;

    const float centerX = bounds.getX() + w / 2.0f;
    const float centerY = bounds.getY() + h / 2.0f;
    const float tickLength = 4.0f;
    const float minorTickLength = 4.0f;
    const int ticksPerDivision = 6;

    const juce::Colour gridColour = juce::Colours::white.withAlpha(0.2f);
    const juce::Colour minorTickColour = juce::Colours::white.withAlpha(0.3f);  
    const juce::Colour strongTickColour = juce::Colours::white.withAlpha(0.7f); 

    //  Dibujar ticks menores en TODA la grilla primero
    g.setColour(minorTickColour);
    for (int i = 0; i <= numVDivs; ++i)
    {
        float yBase = bounds.getY() + i * vStep;
        for (int j = 0; j < ticksPerDivision; ++j)
        {
            float y = yBase + (j * vStep) / (ticksPerDivision - 1);
            for (int k = 0; k <= numHDivs; ++k)
            {
                float x = bounds.getX() + k * hStep;
                g.drawLine(x - minorTickLength / 2.0f, y, x + minorTickLength / 2.0f, y, 1.0f);
            }
        }
    }

    for (int i = 0; i <= numHDivs; ++i)
    {
        float xBase = bounds.getX() + i * hStep;
        for (int j = 0; j < ticksPerDivision; ++j)
        {
            float x = xBase + (j * hStep) / (ticksPerDivision - 1);
            for (int k = 0; k <= numVDivs; ++k)
            {
                float y = bounds.getY() + k * vStep;
                g.drawLine(x, y - minorTickLength / 2.0f, x, y + minorTickLength / 2.0f, 1.0f);
            }
        }
    }

    // Dibujar lineas de cuadrcula (luego de los ticks menores)
    g.setColour(gridColour);
    for (int i = 0; i <= numHDivs; ++i)
    {
        float x = bounds.getX() + i * hStep;
        g.drawLine(x, bounds.getY(), x, bounds.getBottom(), 1.0f);
    }

    for (int i = 0; i <= numVDivs; ++i)
    {
        float y = bounds.getY() + i * vStep;
        g.drawLine(bounds.getX(), y, bounds.getRight(), y, 1.0f);
    }

    //  Ticks fuertes: centro y bordes
    g.setColour(strongTickColour);
    for (int i = 0; i <= numVDivs; ++i)
    {
        float yBase = bounds.getY() + i * vStep;
        for (int j = 0; j < ticksPerDivision; ++j)
        {
            float y = yBase + (j * vStep) / (ticksPerDivision - 1);

            // centro vertical
            g.drawLine(centerX - tickLength / 2.0f, y, centerX + tickLength / 2.0f, y, 1.0f);

            // bordes Y
            g.drawLine(bounds.getX(), y, bounds.getX() + tickLength, y, 1.0f);
            g.drawLine(bounds.getRight() - tickLength, y, bounds.getRight(), y, 1.0f);
        }
    }

    for (int i = 0; i <= numHDivs; ++i)
    {
        float xBase = bounds.getX() + i * hStep;
        for (int j = 0; j < ticksPerDivision; ++j)
        {
            float x = xBase + (j * hStep) / (ticksPerDivision - 1);

            // centro horizontal
            g.drawLine(x, centerY - tickLength / 2.0f, x, centerY + tickLength / 2.0f, 1.0f);

            // bordes X
            g.drawLine(x, bounds.getY(), x, bounds.getY() + tickLength, 1.0f);
            g.drawLine(x, bounds.getBottom() - tickLength, x, bounds.getBottom(), 1.0f);
        }
    }
}

void TimeVisualizer::paint(juce::Graphics& g)
{
    juce::MessageManagerLock mmLock;
    if (!mmLock.lockWasGained()) return;

    constexpr float calibrationFactor = 12.0f; //factor de calibracion para las tensiones reales (CHEQUEAR)

    const float cornerRadius = 8.0f;
    const float borderThickness = 4.0f;
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Colors::PlotSection::background);
    g.fillRoundedRectangle(bounds, cornerRadius);

    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);
    g.reduceClipRegion(clipPath);

    drawGrid(g, bounds);
    g.setColour(Colors::PlotSection::outline);
    g.drawRoundedRectangle(bounds, cornerRadius, borderThickness);

    // ========== ESCALA TEMPORAL ==========
    const float sampleRate = (float)processor.getSampleRate();
    const float secondsPerDiv = processor.params.getHorizontalScaleInSeconds();
    const float totalTime = secondsPerDiv * 10.0f; 
    int displaySamples = static_cast<int>(totalTime * sampleRate);

    // ========== CIRCULAR BUFFER ==========
    juce::AudioBuffer<float> tempBuffer;
    processor.getCircularBuffer().getMostRecentWindow(tempBuffer, displaySamples + 2048); 
    juce::AudioBuffer<float> buffer;

    int numSamples = tempBuffer.getNumSamples();
    if (numSamples < 16) return;

    buffer = std::move(tempBuffer);
    displaySamples = std::min(displaySamples, numSamples);

    const int numChannels = processor.getTotalNumInputChannels();
    const float voltsPerDiv = processor.params.getVerticalScaleInVolts();
    const float pixelsPerDiv = static_cast<float>(getHeight()) / 8.0f;
    const float pixelsPerVolt = (pixelsPerDiv / voltsPerDiv) * calibrationFactor;
    const float centerY = getHeight() / 2.0f;

    int triggerSample = trigger.findTriggerPoint(buffer, 0);
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    // ========== DIBUJO DE LA ONDA ==========
    if (modeDC)
    {
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

        float y = centerY - (maxVal - minVal) * pixelsPerVolt - verticalOffset;
        g.setColour(juce::Colours::limegreen);
        g.drawLine(0.0f, y, (float)getWidth(), y, 2.0f);

        minY = centerY - maxVal * pixelsPerVolt - verticalOffset;
        maxY = centerY - minVal * pixelsPerVolt - verticalOffset;
    }
    else
    {
        juce::Path path;
        float timePerSample = 1.0f / sampleRate;
        float pixelsPerSecond = getWidth() / totalTime;
        int offsetSamples = static_cast<int>(horizontalOffset * totalTime * sampleRate);

        for (int i = 0; i < displaySamples; ++i)
        {
            int sampleIndex = triggerSample + offsetSamples + i;
            while (sampleIndex >= numSamples) sampleIndex -= numSamples;
            while (sampleIndex < 0) sampleIndex += numSamples;

            float sum = 0.0f;
            for (int c = 0; c < numChannels; ++c)
                sum += buffer.getSample(c, sampleIndex);

            float t = i * timePerSample;
            float x = t * pixelsPerSecond;
            float y = centerY - (sum / numChannels) * pixelsPerVolt - verticalOffset;

            if (i == 0) path.startNewSubPath(x, y);
            else        path.lineTo(x, y);

            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }

        g.setColour(Colors::PlotSection::timeResponse);
        g.strokePath(path, juce::PathStrokeType(2.0f));

        // ========== TRIGGER Y FLECHAS ==========
        const float markerSize = 8.0f;
        const float pad = 6.0f;

        float triggerY = centerY - (currentTriggerLevel * pixelsPerVolt) - verticalOffset;
        float refY = centerY - verticalOffset;

        juce::Path trig;
        trig.addTriangle(bounds.getRight() - pad - markerSize, triggerY,
            bounds.getRight() - pad, triggerY - markerSize * 0.6f,
            bounds.getRight() - pad, triggerY + markerSize * 0.6f);
        g.setColour(Colors::PlotSection::triggerMarker);
        g.fillPath(trig);

        juce::Path ref;
        ref.addTriangle(bounds.getX() + pad, refY - markerSize * 0.6f,
            bounds.getX() + pad, refY + markerSize * 0.6f,
            bounds.getX() + pad + markerSize, refY);
        g.setColour(Colors::PlotSection::referenceArrow);
        g.fillPath(ref);
    }

    // ========== Vpp Y FRECUENCIA ==========
    float signalPixels = maxY - minY;
    float signalDivisions = signalPixels / pixelsPerDiv;
    lastVpp = signalDivisions * voltsPerDiv;

    const int currentRange = processor.params.rangeValue;
    const int currentIndex = processor.params.verticalScaleIndex;

    float frequencyHz = -1.0f;
    if (numSamples > 2)
    {
        const float* channelData = buffer.getReadPointer(0);
        int first = -1, second = -1;

        for (int i = 1; i < numSamples; ++i)
        {
            if (channelData[i - 1] < 0.0f && channelData[i] >= 0.0f)
            {
                if (first == -1)
                    first = i;
                else
                {
                    second = i;
                    break;
                }
            }
        }

        if (first != -1 && second != -1)
        {
            int periodSamples = second - first;
            frequencyHz = sampleRate / periodSamples;
        }
    }

    if (currentRange >= 0 && currentRange < verticalScaleByRange.size()
        && currentIndex >= 0 && currentIndex < verticalScaleByRange[currentRange].size())
    {
        juce::String labelLeft = verticalScaleByRange[currentRange][currentIndex].first;
        juce::String labelRight;
        juce::String labelFreq;

        if (lastVpp < 1.0f)
            labelRight = "Vpp: " + juce::String(lastVpp * 1000.0f, 2) + " mV";
        else
            labelRight = "Vpp: " + juce::String(lastVpp, 2) + " V";

        if (frequencyHz > 0.0f)
        {
            if (frequencyHz >= 1000.0f)
                labelFreq = "Freq: " + juce::String(frequencyHz / 1000.0f, 2) + " kHz";
            else
                labelFreq = "Freq: " + juce::String(frequencyHz, 1) + " Hz";
        }

        g.setFont(14.0f);
        g.setColour(juce::Colours::orange.withAlpha(0.8f));
        g.drawText(labelLeft, 8, getHeight() - 24, 100, 20, juce::Justification::left);
        g.drawText(labelRight, getWidth() - 110, getHeight() - 24, 100, 20, juce::Justification::right);
        if (frequencyHz > 0.0f)
            g.drawText(labelFreq, getWidth() - 110, getHeight() - 42, 100, 20, juce::Justification::right);
    }

    // s/div
    float sPerDiv = processor.params.getHorizontalScaleInSeconds();
    juce::String labelTime;

    if (sPerDiv >= 1.0f)
        labelTime = juce::String(sPerDiv, 1) + " s/div";
    else if (sPerDiv >= 0.001f)
        labelTime = juce::String(sPerDiv * 1000.0f, 0) + " ms/div";
    else
        labelTime = juce::String(sPerDiv * 1e6f, 0) + " µs/div";

    // s/div encima de V/div (lado izquierdo)
    g.setFont(14.0f);
    g.setColour(juce::Colours::orange.withAlpha(0.8f));
    g.drawText(labelTime, 8, getHeight() - 42, 100, 20, juce::Justification::left);
}

void TimeVisualizer::timerCallback()
{
    float triggerLevel = processor.getTriggerLevel();
    updateTriggerParameters(triggerLevel, 0.0f, false);

    if (isVisible())
        repaint();
}

void TimeVisualizer::updateTriggerParameters(float level, float offset, bool filterEnabled)
{
    currentTriggerLevel = juce::jlimit(-1.0f, 1.0f, level);
    trigger.setParameters(level, offset, filterEnabled);
}
