#include "FFT.h"

FFT::FFT() : juce::Thread("FFT-Processor")
{
    averager.clear();
}

FFT::~FFT() 
{
}

void FFT::addAudioData(const juce::AudioBuffer<float>& buffer, int startChannel, int numChannels)
{
    if (abstractFifo.getFreeSpace() < buffer.getNumSamples())
        return;

    int start1, block1, start2, block2;
    abstractFifo.prepareToWrite(buffer.getNumSamples(), start1, block1, start2, block2);

    audioFifo.copyFrom(0, start1, buffer.getReadPointer(startChannel), block1);
    if (block2 > 0)
        audioFifo.copyFrom(0, start2, buffer.getReadPointer(startChannel, block1), block2);

    for (int channel = startChannel + 1; channel < startChannel + numChannels; ++channel)
    {
        if (block1 > 0) audioFifo.addFrom(0, start1, buffer.getReadPointer(channel), block1);
        if (block2 > 0) audioFifo.addFrom(0, start2, buffer.getReadPointer(channel, block1), block2);
    }
    abstractFifo.finishedWrite(block1 + block2);
    waitForData.signal();
}

void FFT::setUpFrequencyAnalyzer(int audioFifoSize, float sampleRateToUse)
{
    sampleRate = sampleRateToUse;
    audioFifo.setSize(1, audioFifoSize);
    abstractFifo.setTotalSize(audioFifoSize);

    startThread(juce::Thread::Priority::normal);
}

void FFT::run()
{
    const int fftSize = fft.getSize();

    while (!threadShouldExit())
    {
        if (abstractFifo.getNumReady() >= fftSize)
        {
            fftBuffer.clear();

            int start1, block1, start2, block2;
            abstractFifo.prepareToRead(fftSize, start1, block1, start2, block2);

            if (block1 > 0)
                fftBuffer.copyFrom(0, 0, audioFifo.getReadPointer(0, start1), block1);

            if (block2 > 0)
                fftBuffer.copyFrom(0, block1, audioFifo.getReadPointer(0, start2), block2);

            abstractFifo.finishedRead((block1 + block2) / 2);

            // Normilized Hann windw
            windowing.multiplyWithWindowingTable(fftBuffer.getWritePointer(0), fftSize);

            // FFT magnitudes
            fft.performFrequencyOnlyForwardTransform(fftBuffer.getWritePointer(0));


            // Averaging thread-safe
            juce::ScopedLock lockedForWriting(pathCreationLock);
            averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples(), -1.0f);
            averager.copyFrom(averagerPtr, 0, fftBuffer.getReadPointer(0), averager.getNumSamples(), 1.0f / (averager.getNumSamples() * (averager.getNumChannels() - 1)));
            averager.addFrom(0, 0, averager.getReadPointer(averagerPtr), averager.getNumSamples());

            if (++averagerPtr == averager.getNumChannels())
                averagerPtr = 1;

            newDataAvailable = true;
        }

        if (abstractFifo.getNumReady() < fftSize)
            waitForData.wait(100);
    }
}

bool FFT::checkForNewData()
{
    auto available = newDataAvailable.load();
    newDataAvailable.store(false);
    return available;
}

void FFT::createPath(juce::Path& p, const juce::Rectangle<float> bounds, float minFreq, float dBMin, float dBMax)
{
    p.clear();
    p.preallocateSpace(8 + averager.getNumSamples() * 3);

    juce::ScopedLock lockedForReading(pathCreationLock);
    const auto* fftData = averager.getReadPointer(0);
    const auto factor = bounds.getWidth() / 10.0f;

    p.startNewSubPath(bounds.getX() + factor * indexToX(0.0f, minFreq),
        binToY(fftData[0], bounds, dBMin, dBMax));

    for (int i = 1; i < averager.getNumSamples(); ++i)
    {
        float x = bounds.getX() + factor * indexToX(static_cast<float>(i), minFreq);
        float y = binToY(fftData[i], bounds, dBMin, dBMax);
        p.lineTo(x, y);
    }
}

float FFT::indexToX(float index, float minFreq) const
{
    const auto freq = (sampleRate * index) / fft.getSize();
    return (freq > 0.01f) ? std::log(freq / minFreq) / std::log(2.0f) : 0.0f;
}

float FFT::binToY(float bin, const juce::Rectangle<float> bounds, float dBMin, float dBMax) const
{
    const float valDB = juce::Decibels::gainToDecibels(bin, dBMin);
    return juce::jmap(valDB, dBMin, dBMax, bounds.getBottom(), bounds.getY());
}

std::vector<std::pair<float, float>> FFT::getHarmonicsInDB(int maxHarmonics, float minDB) const
{
    juce::ScopedLock lock(pathCreationLock);

    std::vector<std::pair<float, float>> result;

    const float* magnitudes = averager.getReadPointer(0);
    int fftSize = fft.getSize();
    int numBins = averager.getNumSamples();
    float binHz = sampleRate / fftSize;

    // Find the fundamental: bin with the largest magnitude
    int fundamentalBin = 0;
    float maxMag = 0.0f;

    for (int i = 1; i < numBins; ++i)
    {
        if (magnitudes[i] > maxMag)
        {
            maxMag = magnitudes[i];
            fundamentalBin = i;
        }
    }

    if (maxMag <= 0.0f)
        return result;

    // Add the exact harmonics (k * fundamentalBin)
    for (int k = 1; k <= maxHarmonics; ++k)
    {
        int bin = k * fundamentalBin;
        if (bin >= numBins) break;

        float freq = bin * binHz;
        float gain = magnitudes[bin];
        float dB = juce::Decibels::gainToDecibels(gain, minDB);
        result.push_back({ freq, dB });
    }

    return result;
}