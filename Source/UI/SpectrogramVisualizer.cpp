#include "SpectrogramVisualizer.h"
#include "../Colormaps/viridis_data.inl"
#include "../Colormaps/rocket_data.inl"
#include "../Colormaps/mako_data.inl"
#include "LookAndFeel.h"

constexpr float verticalPaddingFactor = 0.05f;

SpectrogramVisualizer::SpectrogramVisualizer()
    : forwardFFT(fftOrder), window(fftSize, juce::dsp::WindowingFunction<float>::blackman)
{
    spectrogramImage = juce::Image(juce::Image::RGB, 512, 256, true);
    sampleFifo.setSize(1, fftSize * 2);
    sampleFifo.clear();
    setMouseEvents();
    startTimerHz(60);
}

void SpectrogramVisualizer::setSampleRate(double sr)
{
    sampleRate = sr;
    resetFrequencyZoom();
}

void SpectrogramVisualizer::setFrequencyZoom(float minFreq, float maxFreq)
{
    freqZoomMin = juce::jlimit(1.0f, (float)(sampleRate / 2.0), minFreq);
    freqZoomMax = juce::jlimit(freqZoomMin + 1.0f, (float)(sampleRate / 2.0), maxFreq);
    isZoomed = true;
}

void SpectrogramVisualizer::resetFrequencyZoom()
{
    freqZoomMin = 0.0f;
    freqZoomMax = static_cast<float>(sampleRate / 2.0);
    isZoomed = false;
}

void SpectrogramVisualizer::setMouseEvents()
{
    setMouseCursor(juce::MouseCursor::NormalCursor);
    setInterceptsMouseClicks(true, false);
}

void SpectrogramVisualizer::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isMiddleButtonDown() || event.mods.isLeftButtonDown()) {
        isPanning = true;
        lastDragY = event.getPosition().getY();
    }
}

void SpectrogramVisualizer::mouseUp(const juce::MouseEvent&)
{
    isPanning = false;
}

void SpectrogramVisualizer::mouseDrag(const juce::MouseEvent& event)
{
    if (!isZoomed || !isPanning)
        return;

    const int currentY = event.getPosition().getY();
    const int deltaY = lastDragY - currentY;
    lastDragY = currentY;

    const float visibleRange = freqZoomMax - freqZoomMin;
    const float deltaFreq = (deltaY / (float)getHeight()) * visibleRange;

    float newMin = juce::jlimit(0.0f, (float)(sampleRate / 2.0) - visibleRange, freqZoomMin + deltaFreq);
    float newMax = newMin + visibleRange;
    setFrequencyZoom(newMin, newMax);
}

void SpectrogramVisualizer::mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel)
{
    if (event.position.x > 60.0f) return;

    const float zoomAmount = wheel.deltaY * 0.2f;
    if (std::abs(zoomAmount) < 1e-4f) return;

    const float mouseY = event.position.y;
    const float normY = 1.0f - juce::jlimit(0.0f, 1.0f, mouseY / (float)getHeight());
    const float freqAtCursor = freqZoomMin + normY * (freqZoomMax - freqZoomMin);

    float newRange = (freqZoomMax - freqZoomMin) * (1.0f - zoomAmount);
    newRange = juce::jlimit(100.0f, (float)(sampleRate / 2.0), newRange);

    float newMin = juce::jlimit(0.0f, (float)(sampleRate / 2.0 - newRange), freqAtCursor - newRange * normY);
    float newMax = newMin + newRange;
    setFrequencyZoom(newMin, newMax);
}

void SpectrogramVisualizer::paint(juce::Graphics& g)
{
    const float cornerRadius = 8.0f;
    const float borderThickness = 4.0f;
    auto bounds = getLocalBounds().toFloat();

    g.setColour(Colors::PlotSection::background);
    g.fillRoundedRectangle(bounds, cornerRadius);

    juce::Path clipPath;
    clipPath.addRoundedRectangle(bounds, cornerRadius);
    g.reduceClipRegion(clipPath);

    g.drawImage(spectrogramImage, bounds);

    g.setColour(Colors::PlotSection::outline);
    g.drawRoundedRectangle(bounds, cornerRadius, borderThickness);

    g.setColour(juce::Colours::silver.withAlpha(0.8f));
    g.setFont(juce::Font(12.0f, juce::Font::bold));

    std::vector<float> freqLabels;
    const float maxDisplayFreq = static_cast<float>(sampleRate / 2.0);
    for (float base = 10.0f; base <= maxDisplayFreq; base *= 10.0f)
    {
        for (int mult : {1, 2, 5})
        {
            float f = base * mult;
            if (f <= maxDisplayFreq)
                freqLabels.push_back(f);
        }
    }

    float lastY = -1000.0f;
    float minSpacing = 20.0f;

    const float freqRange = freqZoomMax - freqZoomMin;
    const float paddedMin = freqZoomMin - freqRange * verticalPaddingFactor;
    const float paddedMax = freqZoomMax + freqRange * verticalPaddingFactor;
    const float paddedRange = paddedMax - paddedMin;

    for (auto freq : freqLabels)
    {
        if (freq < paddedMin || freq > paddedMax)
            continue;

        float norm = (freq - paddedMin) / paddedRange;
        float y = bounds.getBottom() - norm * bounds.getHeight();

        if (std::abs(y - lastY) < minSpacing)
            continue;

        lastY = y;

        juce::String label;
        if (freq >= 1000.0f)
            label = juce::String(freq / 1000.0f, 1) + " kHz";
        else
            label = juce::String((int)freq) + " Hz";

        g.drawFittedText(label, 4, (int)y - 7, 48, 14, juce::Justification::left, 1);
    }

    // === PLOTBAR de dB ===
    const int plotbarWidth = 16;
    const float plotbarHeight = bounds.getHeight() * 0.4f;
    auto plotbarTop = bounds.getY() + 20.0f; 
    const float plotbarLeft = bounds.getRight() - plotbarWidth - 8.0f;

    for (int y = 0; y < (int)plotbarHeight; ++y)
    {
        float norm = 1.0f - (float)y / (float)plotbarHeight;
        float dB = juce::jmap(norm, -80.0f, 0.0f);
        float colorNorm = juce::jlimit(0.0f, 1.0f, juce::jmap(dB, -100.0f, 0.0f, 0.0f, 1.0f));
        int colorIndex = static_cast<int>(colorNorm * 255.0f);
        const auto& rgb = mako256[colorIndex];
        juce::Colour colour = juce::Colour::fromFloatRGBA(rgb[0], rgb[1], rgb[2], 1.0f);

        g.setColour(colour);
        g.fillRect(juce::Rectangle<float>((float)plotbarLeft, plotbarTop + (float)y, (float)plotbarWidth, 1.0f));
    }

    // === Etiquetas dB ===
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.setColour(juce::Colours::silver.withAlpha(0.9f));
    for (float dBval = 0.0f; dBval >= -80.0f; dBval -= 20.0f)
    {
        float norm = juce::jmap(dBval, -80.0f, 0.0f, 0.0f, 1.0f);
        int y = static_cast<int>(plotbarTop + (1.0f - norm) * plotbarHeight);
        g.drawFittedText(juce::String((int)dBval) + " dB", plotbarLeft - 42, y - 6, 36, 12, juce::Justification::right, 1);
    }

    g.drawFittedText("Time", bounds.getCentreX() - 60, bounds.getBottom() - 20, 60, 14, juce::Justification::centred, 1);
}

void SpectrogramVisualizer::pushBuffer(const juce::AudioBuffer<float>& buffer)
{
    const float* channelData = buffer.getReadPointer(0);
    const int numSamples = buffer.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        sampleFifo.setSample(0, sampleFifoIndex++, channelData[i]);

        if (sampleFifoIndex >= fftSize)
        {
            performFFT();

            for (int j = 0; j < fftSize - hopSize; ++j)
                sampleFifo.setSample(0, j, sampleFifo.getSample(0, j + hopSize));

            sampleFifoIndex = fftSize - hopSize;
        }
    }
}

void SpectrogramVisualizer::performFFT()
{
    const float* fifoData = sampleFifo.getReadPointer(0);

    std::copy(fifoData, fifoData + fftSize, fftData.begin());
    window.multiplyWithWindowingTable(fftData.data(), fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData.data());

    std::vector<float> spectrum(fftSize / 2);
    std::copy(fftData.begin(), fftData.begin() + fftSize / 2, spectrum.begin());

    const juce::ScopedLock lock(spectrumLock);
    if (spectrumQueue.size() >= maxQueueSize)
        spectrumQueue.pop_front();

    spectrumQueue.push_back(std::move(spectrum));
}

void SpectrogramVisualizer::timerCallback()
{
    drawNextLineOfSpectrogram();
    repaint();
}

void SpectrogramVisualizer::drawNextLineOfSpectrogram()
{
    std::vector<float> spectrum;

    {
        const juce::ScopedLock lock(spectrumLock);
        if (spectrumQueue.empty()) return;

        spectrum = std::move(spectrumQueue.front());
        spectrumQueue.pop_front();
    }

    const int imageHeight = spectrogramImage.getHeight();
    const int imageWidth = spectrogramImage.getWidth();

    spectrogramImage.moveImageSection(0, 0, 1, 0, imageWidth - 1, imageHeight);

    const float freqRange = freqZoomMax - freqZoomMin;
    const float paddedMin = freqZoomMin - freqRange * verticalPaddingFactor;
    const float paddedMax = freqZoomMax + freqRange * verticalPaddingFactor;
    const float paddedRange = paddedMax - paddedMin;

    for (int y = 0; y < imageHeight; ++y)
    {
        float normY = 1.0f - (float)y / (float)imageHeight;
        float freq = paddedMin + normY * paddedRange;
        freq = juce::jlimit(0.0f, (float)(sampleRate / 2.0), freq);

        int bin = static_cast<int>(freq * fftSize / sampleRate);
        bin = juce::jlimit(0, fftSize / 2 - 1, bin);

        float magnitude = spectrum[bin];
        float dB = juce::Decibels::gainToDecibels(magnitude, -100.0f);
        float norm = juce::jlimit(0.0f, 1.0f, juce::jmap(dB, -100.0f, 0.0f, 0.0f, 1.0f));

        int colorIndex = static_cast<int>(norm * 255.0f);
        const auto& rgb = mako256[colorIndex];
        juce::Colour colour = juce::Colour::fromFloatRGBA(rgb[0], rgb[1], rgb[2], 1.0f);
        spectrogramImage.setPixelAt(imageWidth - 1, y, colour);
    }
}