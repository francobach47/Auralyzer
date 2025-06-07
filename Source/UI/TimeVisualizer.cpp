#include <JuceHeader.h>
#include "TimeVisualizer.h"
#include "../DSP/SignalAnalysis.h"

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
    const float pixelsPerDivision = getHeight() / 8.0f;
    verticalOffset = offsetDivs * pixelsPerDivision;
    repaint();
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
    // Ingresás el level en divisiones (como setea el pote)
    const float voltsPerDiv = processor.params.getVerticalScaleInVolts();
    const float volts = level * voltsPerDiv; // lo pasás a voltios reales
    const float uncalibrated = volts / processor.getCalibrationFactor(); // lo llevás al dominio de la señal sin calibrar (entre -1 y 1)

    currentTriggerLevel = uncalibrated; // esto se usa para dibujar el triángulo
    trigger.setParameters(uncalibrated, offset, filterEnabled); // esto para detectar el flanco en el dominio de señal
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

    //  Dibujar ticks para la grilla
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

    // Lineas de cuadricula
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
    const float calibrationFactor = processor.getCalibrationFactor();

    const float sampleRate = (float)processor.getSampleRate();
    const float secondsPerDiv = processor.params.getHorizontalScaleInSeconds();
    const float totalTime = secondsPerDiv * 10.0f;
    int displaySamples = static_cast<int>(totalTime * sampleRate);

    juce::AudioBuffer<float> tempBuffer;
    processor.getCircularBuffer().getMostRecentWindow(tempBuffer, displaySamples + 2048);
    if (tempBuffer.getNumSamples() < 16) return;

    juce::AudioBuffer<float> buffer = std::move(tempBuffer);
    displaySamples = std::min(displaySamples, buffer.getNumSamples());

    const float voltsPerDiv = processor.params.getVerticalScaleInVolts();
    const float pixelsPerDiv = getHeight() / 8.0f;
    const float pixelsPerVolt = (pixelsPerDiv / voltsPerDiv) * calibrationFactor;
    const float centerY = getHeight() / 2.0f;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    auto bounds = getLocalBounds().toFloat();
    g.setColour(Colors::PlotSection::background);
    g.fillRoundedRectangle(bounds, 8.0f);

    drawGrid(g, bounds);
    g.setColour(Colors::PlotSection::outline);
    g.drawRoundedRectangle(bounds, 8.0f, 4.0f);

    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();

    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();

    // ========== DIBUJO DE LA ONDA ==========
    if (modeDC)
    {
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
        int triggerSample = trigger.findTriggerPoint(buffer, 0);
        juce::Path path;
        float timePerSample = 1.0f / sampleRate;
        float pixelsPerSecond = getWidth() / totalTime;
        int offsetSamples = static_cast<int>(-horizontalOffset * secondsPerDiv * sampleRate);

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

        // ========== FLECHAS (marcadores visuales) ==========
        const float markerSize = 8.0f;
        const float pad = 6.0f;

        const float refY = centerY - verticalOffset;
        const float trigY = centerY - (currentTriggerLevel * pixelsPerVolt) - verticalOffset;

        juce::Path refArrow;
        refArrow.addTriangle(bounds.getX() + pad, refY - markerSize * 0.6f,
            bounds.getX() + pad, refY + markerSize * 0.6f,
            bounds.getX() + pad + markerSize, refY);
        g.setColour(juce::Colours::yellow);
        g.fillPath(refArrow);

        juce::Path trigArrow;
        trigArrow.addTriangle(bounds.getRight() - pad - markerSize, trigY,
            bounds.getRight() - pad, trigY - markerSize * 0.6f,
            bounds.getRight() - pad, trigY + markerSize * 0.6f);
        g.setColour(juce::Colours::white);
        g.fillPath(trigArrow);

        float xNorm = (horizontalOffset + 5.0f) / 10.0f;
        float xPos = xNorm * (float)getWidth();

        juce::Path hOffsetArrow;
        hOffsetArrow.addTriangle(xPos - markerSize * 0.6f, pad,
            xPos + markerSize * 0.6f, pad,
            xPos, pad + markerSize);
        g.setColour(juce::Colours::yellow);
        g.fillPath(hOffsetArrow);
    }

    // ========== MEDICIONES ==========
    float calibratedVpp = SignalAnalysis::computeVpp(minY, maxY, pixelsPerDiv, voltsPerDiv);
    float rms = SignalAnalysis::computeRMS(buffer, 1.0f);
    float calibratedRMS = processor.getCorrectedVoltage(rms);
    float frequencyHz = SignalAnalysis::computeFrequency(buffer, sampleRate);
    float thdRatio = SignalAnalysis::computeTHD(buffer, sampleRate, 11);
    lastVpp = calibratedVpp;

    const int currentRange = processor.params.rangeValue;
    const int currentIndex = processor.params.verticalScaleIndex;

    juce::String labelLeft, labelDC, labelRMS, labelVpp, labelFreq, labelTHD;

    if (currentRange >= 0 && currentRange < verticalScaleByRange.size()
        && currentIndex >= 0 && currentIndex < verticalScaleByRange[currentRange].size())
    {
        labelLeft = verticalScaleByRange[currentRange][currentIndex].first;
    }

    if (modeDC)
    {
        // Mostrar valor continuo equivalente
        if (calibratedVpp < 1.0f)
            labelVpp = "DC: " + juce::String(calibratedVpp * 1000.0f, 2) + " mV";
        else
            labelVpp = "DC: " + juce::String(calibratedVpp, 2) + " V";
    }
    else
    {
        // Mostrar medidas alterna (Vpp, RMS)
        if (calibratedRMS < 1.0f)
            labelRMS = "RMS: " + juce::String(calibratedRMS * 1000.0f, 2) + " mV";
        else
            labelRMS = "RMS: " + juce::String(calibratedRMS, 2) + " V";

        if (calibratedVpp < 1.0f)
            labelVpp = "Vpp: " + juce::String(calibratedVpp * 1000.0f, 2) + " mV";
        else
            labelVpp = "Vpp: " + juce::String(calibratedVpp, 2) + " V";
    }
    
    if (frequencyHz > 0.0f)
    {
        if (frequencyHz >= 1000.0f)
            labelFreq = "Freq: " + juce::String(frequencyHz / 1000.0f, 2) + " kHz";
        else
            labelFreq = "Freq: " + juce::String(frequencyHz, 1) + " Hz";
    }

    labelTHD = "THD: " + juce::String(thdRatio * 100.0f, 3) + " %";

    g.setFont(14.0f);
    g.setColour(juce::Colours::orange.withAlpha(0.8f));
    g.drawText(labelLeft, 8, getHeight() - 24, 100, 20, juce::Justification::left);

    if (modeDC)
    {
        g.drawText(labelVpp, getWidth() - 110, getHeight() - 24, 100, 20, juce::Justification::right);
    }
    else
    {
        g.drawText(labelTHD, getWidth() - 110, getHeight() - 42, 100, 20, juce::Justification::right);
        g.drawText(labelRMS, getWidth() - 110, getHeight() - 78, 100, 20, juce::Justification::right);
        g.drawText(labelVpp, getWidth() - 110, getHeight() - 60, 100, 20, juce::Justification::right);
        if (!labelFreq.isEmpty())
            g.drawText(labelFreq, getWidth() - 110, getHeight() - 24, 100, 20, juce::Justification::right);
    }

    float sPerDiv = processor.params.getHorizontalScaleInSeconds();
    juce::String labelTime;
    if (sPerDiv >= 1.0f)
        labelTime = juce::String(sPerDiv, 1) + " s/div";
    else if (sPerDiv >= 0.001f)
        labelTime = juce::String(sPerDiv * 1000.0f, 0) + " ms/div";
    else
        labelTime = juce::String(sPerDiv * 1e6f, 0) + " µs/div";

    g.drawText(labelTime, 100, getHeight() - 24, 100, 20, juce::Justification::left);
}
