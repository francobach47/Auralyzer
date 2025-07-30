#pragma once
#include <JuceHeader.h>
#include <deque>

class SpectrogramVisualizer : public juce::Component,
    private juce::Timer
{
public:
    SpectrogramVisualizer();

    void pushBuffer(const juce::AudioBuffer<float>& buffer);
    void paint(juce::Graphics&) override;
    void setSampleRate(double sr);

    void setFrequencyZoom(float minFreq, float maxFreq); // Zoom in
    void resetFrequencyZoom();                           // Zoom out (ver todo)

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

protected:
    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

private:
    void timerCallback() override;
    void drawNextLineOfSpectrogram();
    void performFFT();
    void setMouseEvents();

    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder;
    static constexpr int hopSize = fftSize / 8;
    static constexpr int maxQueueSize = 8;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;

    juce::AudioBuffer<float> sampleFifo;
    int sampleFifoIndex = 0;

    std::array<float, fftSize * 2> fftData{};
    juce::Image spectrogramImage;

    double sampleRate = 44100.0;

    std::deque<std::vector<float>> spectrumQueue;
    juce::CriticalSection spectrumLock;

    // Zoom
    float freqZoomMin = 0.0f;
    float freqZoomMax = 22050.0f;
    bool isZoomed = false;

    // Interacción
    bool isPanning = false;
    int lastDragY = 0;
};