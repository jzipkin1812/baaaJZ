#include "BaaaProcessor.h"
#include "BaaaEditor.h"
#include "DSP/utility.h"
#include <iostream>

// INPUT HANDLING
juce::AudioProcessorValueTreeState::ParameterLayout
BaaaPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Frequency (Hz)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "shiftAmt",
        "Shift Amount (ct)",
        juce::NormalisableRange<float> (-12.0f, 12.0f, 0.5f),
        0.0f
    ));

    // Output gain (dB)
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGain",
        "Output Gain",
        juce::NormalisableRange<float>(-40.0f, 6.0f, 0.01f),
        -20.0f
    ));

    // Superpositions
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "upCount",
        "Superpositions Up",
        juce::NormalisableRange<float>(0.0f, 10.0f, 1.0f),
        0.5f
    ));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "downCount",
        "Superpositions Down",
        juce::NormalisableRange<float>(0.0f, 10.0f, 1.0f),
        0.5f
    ));

    return { params.begin(), params.end() };
}


//==============================================================================
BaaaPluginAudioProcessor::BaaaPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                       apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    for(auto& s : shifters)
        s = PhaseVocoderPitchShifter();
}

BaaaPluginAudioProcessor::~BaaaPluginAudioProcessor()
{
}

//==============================================================================
const juce::String BaaaPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BaaaPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BaaaPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BaaaPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BaaaPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BaaaPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BaaaPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BaaaPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String BaaaPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void BaaaPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void BaaaPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    const size_t numChannels = (size_t)getTotalNumOutputChannels();

    shifters.clear();
    shifters.resize (numChannels);

    for (auto& shifter : shifters)
        shifter.prepare (sampleRate);
}

void BaaaPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool BaaaPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void BaaaPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels  = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    for (int ch = totalNumInputChannels; ch < totalNumOutputChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    const float gainDb =
        apvts.getRawParameterValue ("outputGain")->load();
    const float gainLinear =
        juce::Decibels::decibelsToGain (gainDb);

    const float semitones =
        apvts.getRawParameterValue ("shiftAmt")->load();

    const float pitchRatio = std::pow (2.0f, semitones / 12.0f);


    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        auto& shifter = shifters[(size_t)(ch)];

        shifter.setPitchRatio (pitchRatio);

        for (int i = 0; i < numSamples; ++i)
        {
            const float dry = data[i];
            const float wet = shifter.processSample (dry);

            data[i] = (wet) * gainLinear;
        }
    }
}

//==============================================================================
bool BaaaPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BaaaPluginAudioProcessor::createEditor()
{
    return new BaaaPluginAudioProcessorEditor (*this);
}

//==============================================================================
// this boiler plate XML saver was copied from
// https://github.com/kybr/Badass-Toy/blob/main/PluginProcessor.cpp
// thanks
void BaaaPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BaaaPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BaaaPluginAudioProcessor();
}
