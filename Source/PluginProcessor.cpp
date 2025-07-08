#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
OscilloscopeAudioProcessor::OscilloscopeAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
    params(apvts) 
{
    //serialDevice.init(kSerialPortName); ----- ESTO SE BORRA
}

OscilloscopeAudioProcessor::~OscilloscopeAudioProcessor()
{
    frequencyAnalyzer.stopThread(1000);
}

//==============================================================================
const juce::String OscilloscopeAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OscilloscopeAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OscilloscopeAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OscilloscopeAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OscilloscopeAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OscilloscopeAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OscilloscopeAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OscilloscopeAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OscilloscopeAudioProcessor::getProgramName (int index)
{
    return {};
}

void OscilloscopeAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OscilloscopeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    const int bufferSeconds = 10; // capacidad total (10 s)
    const int bufferSize = static_cast<int>(sampleRate * bufferSeconds);
    circularBuffer.prepare(getTotalNumInputChannels(), bufferSize);


    frequencyAnalyzer.setUpFrequencyAnalyzer(int(sampleRate), sampleRate);

    // Inicializamos el incremento de fase para 1 kHz
    phase = 0.0;
    phaseIncrement = juce::MathConstants<double>::twoPi * 1000.0 / sampleRate;

}

void OscilloscopeAudioProcessor::releaseResources()
{
    frequencyAnalyzer.stopThread(1000);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OscilloscopeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& mainIn = layouts.getChannelSet(true, 0); // Input
    const auto& mainOut = layouts.getChannelSet(false, 0); // Output

    return mainIn == juce::AudioChannelSet::stereo()
        && mainOut == juce::AudioChannelSet::stereo();
}
#endif

void OscilloscopeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, 
                              [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    params.update();

    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    static bool lastFrequencyMode = false;
    bool currentFrequencyMode = apvts.getRawParameterValue(plotModeParamID.getParamID())->load() > 0.5f;

    auto numOutputChannels = getTotalNumOutputChannels();
    auto numInputChannels = getTotalNumInputChannels();

    bool isFrequencyMode = apvts.getRawParameterValue(plotModeParamID.getParamID())->load() > 0.5f;
        
    if (isFrequencyMode) {
        frequencyAnalyzer.addAudioData(buffer, 0, numOutputChannels);
    }
    else {
        circularBuffer.pushBlock(buffer);
    }

    if (sineEnabled)
    {
        auto numSamples = buffer.getNumSamples();
        auto numChannels = buffer.getNumChannels();

        float amplitude = (params.modeValue == 1) ? 2*0.412f : 0.4205f; // 800 mVpp DC, 400 mVpp AC balanceado

        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);

            for (int i = 0; i < numSamples; ++i)
            {
                float sample = std::sin(phase) * amplitude;
                channelData[i] = sample;
                phase += phaseIncrement;

                if (phase >= juce::MathConstants<double>::twoPi)
                    phase -= juce::MathConstants<double>::twoPi;
            }
        }
    }
    else
    {
        buffer.clear(); //  Si el tono no está activo, se muetea la salida
    }

}

//==============================================================================
bool OscilloscopeAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* OscilloscopeAudioProcessor::createEditor()
{
    return new OscilloscopeAudioProcessorEditor (*this);
}

//==============================================================================
void OscilloscopeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state = apvts.copyState();

    // Guardamos los factores de calibración y sus rangos como propiedades
    state.setProperty("calibrationFactorAC", calibrationFactorAC, nullptr);
    state.setProperty("calibrationFactorDC", calibrationFactorDC, nullptr);
    state.setProperty("calibrationRangeAC", calibrationRangeAC, nullptr);
    state.setProperty("calibrationRangeDC", calibrationRangeDC, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void OscilloscopeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xml);

        if (state.hasProperty("calibrationFactorAC"))
            calibrationFactorAC = static_cast<float>(state["calibrationFactorAC"]);

        if (state.hasProperty("calibrationFactorDC"))
            calibrationFactorDC = static_cast<float>(state["calibrationFactorDC"]);
       
        if (state.hasProperty("calibrationRangeAC"))
            calibrationRangeAC = static_cast<int>(state["calibrationRangeAC"]);

        if (state.hasProperty("calibrationRangeDC"))
            calibrationRangeDC = static_cast<int>(state["calibrationRangeDC"]);

        apvts.replaceState(state);
    }
}

void OscilloscopeAudioProcessor::createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float dBMin, float dBMax)
{
    frequencyAnalyzer.createPath(p, bounds.toFloat(), 20.0f, dBMin, dBMax);
}

bool OscilloscopeAudioProcessor::checkForNewAnalyserData()
{
    return frequencyAnalyzer.checkForNewData();
}

void OscilloscopeAudioProcessor::startLevelCalibration()
{
    juce::AudioBuffer<float> buffer;
    circularBuffer.getMostRecentWindow(buffer, 1024);

    float minVal = std::numeric_limits<float>::max();
    float maxVal = std::numeric_limits<float>::lowest();

    for (int c = 0; c < buffer.getNumChannels(); ++c)
    {
        const float* data = buffer.getReadPointer(c);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float v = data[i];
            minVal = std::min(minVal, v);
            maxVal = std::max(maxVal, v);
        }
    }

    float measuredVpp = maxVal - minVal;
    float expectedVpp = (params.modeValue == 1) ? 1.0f : 1.0f; //el primero es DC el segundo AC
    float newFactor = expectedVpp / measuredVpp;

    if (params.modeValue == 1)
        calibrationFactorDC = newFactor;
    else
        calibrationFactorAC = newFactor;

    // Guardar el rango usado en calibración
    calibrationRange = params.rangeValue;
}


float OscilloscopeAudioProcessor::getCalibrationFactor() const
{
    float factorActual = rangeCompensationFactors[params.rangeValue];

    if (params.modeValue == 0) // AC
    {
        float factorCalibrado = rangeCompensationFactors[calibrationRangeAC];
        float factorRelativo = factorActual / factorCalibrado;
        return calibrationFactorAC * factorRelativo;
    }
    else // DC
    {
        float factorCalibrado = rangeCompensationFactors[calibrationRangeDC];
        float factorRelativo = factorActual / factorCalibrado;
        return calibrationFactorDC * factorRelativo;
    }
}

float OscilloscopeAudioProcessor::getCorrectedVoltage(float value) const
{
    return value * getCalibrationFactor();
}

void OscilloscopeAudioProcessor::setSineEnabled(bool enabled)
{
    sineEnabled = enabled;
}

std::vector<std::pair<float, float>> OscilloscopeAudioProcessor::getHarmonicLabels() const
{
    return frequencyAnalyzer.getHarmonicsInDB(6, -80.0f);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OscilloscopeAudioProcessor();
}