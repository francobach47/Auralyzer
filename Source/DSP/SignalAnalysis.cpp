#include "SignalAnalysis.h"
#include <juce_dsp/juce_dsp.h>


float SignalAnalysis::computeRMS(const juce::AudioBuffer<float>& buffer, float calibrationFactor)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    if (numChannels == 0 || numSamples == 0) return 0.0f;

    double sumSquares = 0.0;
    for (int c = 0; c < numChannels; ++c)
    {
        const float* data = buffer.getReadPointer(c);
        for (int i = 0; i < numSamples; ++i)
        {
            float v = data[i] * calibrationFactor; 
            sumSquares += v * v;
        }
    }

    float rmsVolts = std::sqrt(sumSquares / (numSamples * numChannels));
    return rmsVolts;
}


float SignalAnalysis::computeVpp(float minY, float maxY, float pixelsPerDiv, float voltsPerDiv)
{
    float signalPixels = maxY - minY;
    float signalDivisions = signalPixels / pixelsPerDiv;
    return signalDivisions * voltsPerDiv;
}


float SignalAnalysis::computeFrequency(const juce::AudioBuffer<float>& buffer, float sampleRate)
{
    const int numSamples = buffer.getNumSamples();
    if (numSamples < 2) return -1.0f;

    const float* channelData = buffer.getReadPointer(0);
    int first = -1, second = -1;

    for (int i = 1; i < numSamples; ++i)
    {
        if (channelData[i - 1] < 0.0f && channelData[i] >= 0.0f)
        {
            if (first == -1) first = i;
            else { second = i; break; }
        }
    }

    if (first != -1 && second != -1)
    {
        int periodSamples = second - first;
        return sampleRate / periodSamples;
    }

    return -1.0f;
}

float SignalAnalysis::computeTHD(const juce::AudioBuffer<float>& buffer, float sampleRate, int fftOrder, int /*unused*/)
{
    const int fftSize = 1 << fftOrder;
    if (buffer.getNumSamples() < fftSize) return 0.0f;

    juce::dsp::FFT fft(fftOrder);
    std::vector<float> fftData(2 * fftSize, 0.0f);

    const float* data = buffer.getReadPointer(0);
    for (int i = 0; i < fftSize; ++i)
        fftData[2 * i] = data[i];  // canal 0, parte real

    juce::dsp::WindowingFunction<float> window(fftSize, juce::dsp::WindowingFunction<float>::hann, true);
    window.multiplyWithWindowingTable(fftData.data(), fftSize);

    fft.performRealOnlyForwardTransform(fftData.data());

    std::vector<float> magnitudes(fftSize / 2, 0.0f);
    for (int i = 0; i < fftSize / 2; ++i)
    {
        float re = fftData[2 * i];
        float im = fftData[2 * i + 1];
        magnitudes[i] = std::sqrt(re * re + im * im);
    }

    magnitudes[0] = 0.0f; // ignorar DC

    // Buscar la fundamental (bin con mayor magnitud)
    auto it = std::max_element(magnitudes.begin(), magnitudes.end());
    int fundamentalBin = std::distance(magnitudes.begin(), it);
    float fundamental = *it;
    if (fundamental == 0.0f) return 0.0f;

    float threshold = fundamental * std::pow(10.0f, -60.0f / 20.0f); // -60 dB

    float sumHarmonicsSq = 0.0f;
    for (int k = 2;; ++k)
    {
        int bin = k * fundamentalBin;
        if (bin >= magnitudes.size()) break;

        float mag = magnitudes[bin];
        if (mag < threshold && k > 5) break;
        DBG("Harmonic k = " << k << ", bin = " << bin << ", magnitude = " << mag << (mag < threshold ? " < threshold" : ""));

        sumHarmonicsSq += mag * mag;
    }

    DBG("Fundamental bin: " << fundamentalBin << ", magnitude: " << fundamental);
    DBG("Threshold (-60 dB): " << threshold);

    return std::sqrt(sumHarmonicsSq) / fundamental;
}