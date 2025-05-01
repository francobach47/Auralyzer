#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OscilloscopeAudioProcessor::OscilloscopeAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), false)
                       ),
    params(apvts)
{
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
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = juce::uint32(getTotalNumOutputChannels());

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

    auto numOutputChannels = getTotalNumOutputChannels();
    auto numInputChannels = getTotalNumInputChannels();

    bool isFrequencyMode = apvts.getRawParameterValue(plotModeParamID.getParamID())->load() > 0.5f;

    if (isFrequencyMode) {
        if (getActiveEditor() != nullptr) {
            frequencyAnalyzer.addAudioData(buffer, 0, numOutputChannels);
        }
    }
    else {
        audioTimeBuffer.makeCopyOf(buffer);
        int latencySamples = getLatencySamples();
        buffer.applyGainRamp(0, latencySamples, 0.0f, 1.0f);    
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
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
}

void OscilloscopeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
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