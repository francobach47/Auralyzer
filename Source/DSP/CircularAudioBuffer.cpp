#include "CircularAudioBuffer.h"

void CircularAudioBuffer::prepare(int numChannels, int capacitySamples)
{
    capacity = capacitySamples;
    writePos = 0;
    storedSamples = 0;
    buffer.setSize(numChannels, capacity, false, true, true);
    buffer.clear();
}

void CircularAudioBuffer::pushBlock(const juce::AudioBuffer<float>& input)
{
    const int numChannels = juce::jmin(buffer.getNumChannels(), input.getNumChannels());
    const int blockSamples = input.getNumSamples();

    for (int i = 0; i < blockSamples; ++i)
    {
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.setSample(ch, writePos, input.getSample(ch, i));

        writePos = (writePos + 1) % capacity;
    }

    storedSamples = juce::jmin(storedSamples + blockSamples, capacity);
}

void CircularAudioBuffer::getMostRecentWindow(juce::AudioBuffer<float>& out, int numSamples) const
{
    const int available = juce::jmin(numSamples, storedSamples);
    const int numChannels = buffer.getNumChannels();

    out.setSize(numChannels, available, false, true, true);

    int start = (writePos - available + capacity) % capacity;
    int firstPart = juce::jmin(capacity - start, available);
    int secondPart = available - firstPart;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* dst = out.getWritePointer(ch);
        const float* src = buffer.getReadPointer(ch);

        std::memcpy(dst, src + start, sizeof(float) * firstPart);
        if (secondPart > 0)
            std::memcpy(dst + firstPart, src, sizeof(float) * secondPart);
    }
}
