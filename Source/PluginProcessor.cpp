#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
OscilloscopeAudioProcessor::OscilloscopeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

OscilloscopeAudioProcessor::~OscilloscopeAudioProcessor()
{
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
void OscilloscopeAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void OscilloscopeAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OscilloscopeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{   // TODO: Change to mono and stereo output
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}
#endif

void OscilloscopeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels  = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();

    for (auto i = numInputChannels; i < numOutputChannels; ++i) {
        buffer.clear(i, 0, numSamples);
    }

    //bool bypassed = apvts.getRawParameterValue("ByPass")->load();
    bool bypassed = false;

    float* channelL = buffer.getWritePointer(0);
    float* channelR = buffer.getWritePointer(1);

    //for (int sample = 0; sample < numSamples; sample++) {
    //    float sampleL = channelL[sample];
    //    float sampleR = channelR[sample];

    //    sampleL = fft[0].processSample(sampleL, bypassed);
    //    sampleR = fft[1].processSample(sampleR, bypassed);

    //    channelL[sample] = sampleL;
    //    channelR[sample] = sampleR;
    //}
    
    /*
    // Processing the entire block at once:
    for (int channel = 0; channel < numInputChannels; ++channel) {
        auto* channelData = buffer.getWritePointer(channel);
        fft[channel].processBlock(channelData, numSamples, bypassed);
    }
    */
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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OscilloscopeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OscilloscopeAudioProcessor();
}
