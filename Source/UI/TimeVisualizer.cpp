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
    const juce::Colour minorTickColour = juce::Colours::white.withAlpha(0.3f);  //  ahora ms visible
    const juce::Colour strongTickColour = juce::Colours::white.withAlpha(0.7f); //  contraste fuerte

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

    constexpr float calibrationFactor = 12.0f; // solo para visualizacion

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

    const auto& buffer = processor.getAudioBuffer();
    const int numChannels = processor.getTotalNumInputChannels();
    const int numSamples = buffer.getNumSamples();

    const float voltsPerDiv = processor.params.getVerticalScaleInVolts();
    const float pixelsPerDiv = static_cast<float>(getHeight()) / 8.0f;
    const float pixelsPerVolt = (pixelsPerDiv / voltsPerDiv) * calibrationFactor;
    const float centerY = getHeight() / 2.0f;

    int triggerSample = trigger.findTriggerPoint(buffer, 0);
    int displaySamples = juce::jlimit(16, numSamples - triggerSample, static_cast<int>(numSamples / horizontalScale));

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

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

        float y = centerY - (maxVal - minVal) * pixelsPerVolt - verticalOffset;
        g.setColour(juce::Colours::limegreen);
        g.drawLine(0.0f, y, (float)getWidth(), y, 2.0f);

        minY = centerY - maxVal * pixelsPerVolt - verticalOffset;
        maxY = centerY - minVal * pixelsPerVolt - verticalOffset;
    }
    else
    {
        juce::Path path;
        float sampleRate = (float)processor.getSampleRate();
        float timePerSample = 1.0f / sampleRate;
        float totalTime = displaySamples * timePerSample * horizontalScale;
        float pixelsPerSecond = getWidth() / totalTime;

        for (int i = 0; i < displaySamples; ++i)
        {
            int sampleIndex = triggerSample + i;
            if (sampleIndex >= numSamples) break;

            float sum = 0.0f;
            for (int c = 0; c < numChannels; ++c)
                sum += buffer.getSample(c, sampleIndex);

            float t = i * timePerSample * horizontalScale; // tiempo real con escala
            float timeOffset = horizontalOffset * totalTime; // desplazamiento en segundos
            float x = (t + timeOffset) * pixelsPerSecond;
            float y = centerY - (sum / numChannels) * pixelsPerVolt - verticalOffset;

            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);

            minY = std::min(minY, y);
            maxY = std::max(maxY, y);
        }


        g.setColour(Colors::PlotSection::timeResponse);
        g.strokePath(path, juce::PathStrokeType(2.0f));

        // ===============================================================
        //  Marcadores de referencia y trigger
        // ===============================================================

        const float markerSize = 8.0f;      // ancho del tringulo
        const float pad = 6.0f;      // margen desde el borde

        // Y del TRIGGER  (mismo clculo que la forma de onda)
        float triggerY = centerY - (currentTriggerLevel * pixelsPerVolt) - verticalOffset;

        // Y de la REFERENCIA (0 V desplazado: lnea central con offset)
        float refY = centerY - verticalOffset;

        // ---------- Tringulo de TRIGGER ------------------
        {
            float x0 = bounds.getRight() - pad - markerSize;   // punta en x0
            juce::Path trig;
            trig.addTriangle(x0, triggerY,               //  punta
                x0 + markerSize, triggerY - markerSize * 0.6f,
                x0 + markerSize, triggerY + markerSize * 0.6f);

            g.setColour(Colors::PlotSection::triggerMarker);
            g.fillPath(trig);
        }

        // ---------- Flecha de REFERENCIA -------------
        {
            float x0 = bounds.getX() + pad;                      // base en el borde
            juce::Path ref;
            ref.addTriangle(x0, refY - markerSize * 0.6f,
                x0, refY + markerSize * 0.6f,
                x0 + markerSize, refY);             //  punta

            g.setColour(Colors::PlotSection::referenceArrow);
            g.fillPath(ref);
        }

    }

    float signalPixels = maxY - minY;
    float signalDivisions = signalPixels / pixelsPerDiv;
    lastVpp = signalDivisions * voltsPerDiv; // sin factor de calibracion en el calculo

    const int currentRange = processor.params.rangeValue;
    const int currentIndex = processor.params.verticalScaleIndex;

    // === Calcular frecuencia por cruce por cero ===
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
            float sampleRate = (float)processor.getSampleRate();
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
        {
            g.drawText(labelFreq, getWidth() - 110, getHeight() - 42, 100, 20, juce::Justification::right);
        }
    }
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
