#include "FFTProcessor.h"

FFTProcessor::FFTProcessor() :
	fft(fftOrder),
	window(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)
{
	// Note that the window is of length `fftSize + 1` because JUCE's windows
	// are symmetrical, which is wrong for overlap-add processing. To make the
	// window periodic, set size to 1025 but only use the first 1024 samples.
}

void FFTProcessor::reset()
{
	count = 0;
	pos = 0;
	
	// Zero out the circular buffers.
	std::fill(inputFifo.begin(), inputFifo.end(), 0.0f);
	std::fill(outputFifo.begin(), outputFifo.end(), 0.0f);
}

float FFTProcessor::processSample(float sample, bool bypassed)
{
	// Places the new sample into the input FIFO 
	// and performs the FFT processing every hopSize. 
	// It also reads from the output FIFO and returns that value.

	// Push the new sample value into the input FIFO
	inputFifo[pos] = sample;

	// Read the output value from the output FIFO.
	// Set the position of the output FIFO back 
	// to zero to add IFFT result to it later.
	float outputSample = outputFifo[pos];
	outputFifo[pos] = 0.0f;

	// Advance the FIFO index and wrap around if necessary.
	pos += 1;
	if (pos == fftSize) {
		pos = 0;
	}

	// Process the FFT frame once we've collected hopSize samples.
	count += 1;
	if (count == hopSize) {
		count = 0;
		processFrame(bypassed);
	}

	return outputSample;
}

void FFTProcessor::processFrame(bool bypassed)
{
	const float* inputPtr = inputFifo.data();
	float* fftPtr = fftData.data();

	// Copy the input FIFO into the FFT working space in two parts.
	std::memcpy(fftPtr, inputPtr + pos, (fftSize - pos) * sizeof(float));
	if (pos > 0) {
		std::memcpy(fftPtr + fftSize - pos, inputPtr, pos * sizeof(float));
	}

	// Apply the window to avoid spectral leakage.
	window.multiplyWithWindowingTable(fftPtr, fftSize);

	if (!bypassed) {
		fft.performRealOnlyForwardTransform(fftPtr, true); // FFT
		processSpectrum(fftPtr, numBins);                  // Do stuff w/ FFT data
		fft.performRealOnlyInverseTransform(fftPtr);       // IFFT
	}

	// Apply the window again for resynthesis.
	window.multiplyWithWindowingTable(fftPtr, fftSize);

	// Scale down the output samples because of the overlapping windows.
	for (int i = 0; i < fftSize; ++i) {
		fftPtr[i] *= windowCorrection;
	}

	// Add ITFT results to the output FIFO.
	for (int i = 0; i < pos; ++i) {
		outputFifo[i] += fftData[i + fftSize - pos];
	}
	for (int i = 0; i < fftSize - pos; ++i) {
		outputFifo[i + pos] += fftData[i];
	}
}

void FFTProcessor::processSpectrum(float* data, int numBins)
{
	// The spectrum data is floats organized as [re, im, re, im, ...]
	// but it's easier to deal with this as std::complex values.
	auto* cdata = reinterpret_cast<std::complex<float>*>(data);

	for (int i = 0; i < numBins; ++i) {
		float magnitude = std::abs(cdata[i]);
		float phase = std::arg(cdata[i]);

		// This is where spectral processing stuff happens.

		// Convert magnitude and phase back into a complex number.
		cdata[i] = std::polar(magnitude, phase);
	}
}