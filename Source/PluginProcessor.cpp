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
    //outputAnalyzer.stopThread(1000);
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
    //sampleRate = newSampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;


    //spec.maximumBlockSize = juce::uint32(newSamplesPerBlock);
    //spec.numChannels = juce::uint32(getTotalNumOutputChannels());

    //outputAnalyzer.setUpFrequencyAnalyzer(int(sampleRate), float(sampleRate));
}

void OscilloscopeAudioProcessor::releaseResources()
{
    //outputAnalyzer.stopThread(1000);
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
    juce::ScopedNoDenormals noDenormals;
    juce::ignoreUnused(midiMessages);

    auto numOutputChannels = getTotalNumOutputChannels();

    //if (getActiveEditor() != nullptr) {
    //    outputAnalyzer.addAudioData(buffer, 0, numOutputChannels);
    //}
}

//==============================================================================
bool OscilloscopeAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OscilloscopeAudioProcessor::createEditor()
{
    return new OscilloscopeAudioProcessorEditor (*this);
}

//==============================================================================
void OscilloscopeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.copyState().createXml(), destData);
    //DBG(apvts.copyState().toXmlString());
}

void OscilloscopeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType())) {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

//bool OscilloscopeAudioProcessor::checkForNewAnalyserData()
//{
//    return outputAnalyzer.checkForNewData();
//}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OscilloscopeAudioProcessor();
}