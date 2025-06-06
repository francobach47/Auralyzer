#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
OscilloscopeAudioProcessor::OscilloscopeAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), false)
                       ),
    params(apvts) //serialDevice() --- ESTO SE BORRA
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
    const int bufferSeconds = 10; // capacidad total (10 s por ejemplo)
    const int bufferSize = static_cast<int>(sampleRate * bufferSeconds);
    circularBuffer.prepare(getTotalNumInputChannels(), bufferSize);


    frequencyAnalyzer.setUpFrequencyAnalyzer(int(sampleRate), sampleRate);
}

void OscilloscopeAudioProcessor::releaseResources()
{
    frequencyAnalyzer.stopThread(1000);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OscilloscopeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::disabled();
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
void OscilloscopeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state = apvts.copyState();

    // Guardamos el calibrationFactor como propiedad
    state.setProperty("calibrationFactor", calibrationFactor, nullptr);

    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}


void OscilloscopeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        juce::ValueTree state = juce::ValueTree::fromXml(*xml);

        if (state.hasProperty("calibrationFactor"))
            calibrationFactor = static_cast<float>(state["calibrationFactor"]);

        apvts.replaceState(state);
    }
}

void OscilloscopeAudioProcessor::createAnalyserPlot(juce::Path& p, const juce::Rectangle<int> bounds, float minFreq)
{
    frequencyAnalyzer.createPath(p, bounds.toFloat(), minFreq);
}

bool OscilloscopeAudioProcessor::checkForNewAnalyserData()
{
    return frequencyAnalyzer.checkForNewData();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OscilloscopeAudioProcessor();
}

void OscilloscopeAudioProcessor::startLevelCalibration()
{
    float salidaHardware = 0.4f;
    float entradaReal = 1.0f;

    float measuredVpp = circularBuffer.computeLastVpp();

    if (measuredVpp > 0.0f)
    {
        calibrationFactor = (entradaReal / salidaHardware) * (salidaHardware / measuredVpp);
        calibrationRange = 2;  // Forzamos calibración para el rango 1 V – 10 V
        DBG("Nuevo calibrationFactor: " << calibrationFactor);
    }
}


float OscilloscopeAudioProcessor::getCalibrationFactor() const
{
    float factorActual = rangeCompensationFactors[params.rangeValue];
    float factorCalibrado = rangeCompensationFactors[calibrationRange];
    float factorRelativo = factorActual / factorCalibrado;
    return calibrationFactor * factorRelativo;
}


float OscilloscopeAudioProcessor::getCorrectedVoltage(float value) const
{
    return value * getCalibrationFactor();
}

