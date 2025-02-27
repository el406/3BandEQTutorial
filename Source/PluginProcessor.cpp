/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
_3BandEQTutorialAudioProcessor::_3BandEQTutorialAudioProcessor()
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

_3BandEQTutorialAudioProcessor::~_3BandEQTutorialAudioProcessor()
{
}

//==============================================================================
const juce::String _3BandEQTutorialAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool _3BandEQTutorialAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool _3BandEQTutorialAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool _3BandEQTutorialAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double _3BandEQTutorialAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int _3BandEQTutorialAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int _3BandEQTutorialAudioProcessor::getCurrentProgram()
{
    return 0;
}

void _3BandEQTutorialAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String _3BandEQTutorialAudioProcessor::getProgramName (int index)
{
    return {};
}

void _3BandEQTutorialAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void _3BandEQTutorialAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    //configuring AudioChains
    juce::dsp::ProcessSpec spec;

    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;

    leftChain.prepare(spec);
    rightChain.prepare(spec); 

    auto chainSettings = getChainSettings(parameterManager);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, chainSettings.peakFreq, chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels)
    );
    

    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;

    


}

void _3BandEQTutorialAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool _3BandEQTutorialAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void _3BandEQTutorialAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto chainSettings = getChainSettings(parameterManager);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(), chainSettings.peakFreq, chainSettings.peakQuality,
        juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels)
    );
    *leftChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;
    *rightChain.get<ChainPositions::Peak>().coefficients = *peakCoefficients;

    auto cutCoeffecients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(),
        (chainSettings.lowCutSlope + 1) * 2);
    auto& leftLowCut = leftChain.get<ChainPositions::LowCut>();
    auto& rightLowCut = rightChain.get<ChainPositions::LowCut>();

    // bypass audio chains
    leftLowCut.setBypassed<0>(true);
    leftLowCut.setBypassed<1>(true);
    leftLowCut.setBypassed<2>(true);
    leftLowCut.setBypassed<3>(true);
    rightLowCut.setBypassed<0>(true);
    rightLowCut.setBypassed<1>(true);
    rightLowCut.setBypassed<2>(true);
    rightLowCut.setBypassed<3>(true);

    // switch statement for easier acess (LEFT)
    switch (chainSettings.lowCutSlope)
    {
    case SLOPE_12:
        *leftLowCut.get<0>().coefficients = *cutCoeffecients[0];
        leftLowCut.setBypassed<0>(false);
        break;
    case SLOPE_24:
        *leftLowCut.get<0>().coefficients = *cutCoeffecients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoeffecients[1];
        leftLowCut.setBypassed<1>(false);
        break;
    case SLOPE_36:
        *leftLowCut.get<0>().coefficients = *cutCoeffecients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoeffecients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoeffecients[2];
        leftLowCut.setBypassed<2>(false);
        break;
    case SLOPE_48:
        *leftLowCut.get<0>().coefficients = *cutCoeffecients[0];
        leftLowCut.setBypassed<0>(false);
        *leftLowCut.get<1>().coefficients = *cutCoeffecients[1];
        leftLowCut.setBypassed<1>(false);
        *leftLowCut.get<2>().coefficients = *cutCoeffecients[2];
        leftLowCut.setBypassed<2>(false);
        *leftLowCut.get<3>().coefficients = *cutCoeffecients[3];
        leftLowCut.setBypassed<3>(false);
        break;
    }


    // switch statement for easier acess
    switch (chainSettings.lowCutSlope)
    {
    case SLOPE_12:
        *rightLowCut.get<0>().coefficients = *cutCoeffecients[0];
        rightLowCut.setBypassed<0>(false);
        break;

    case SLOPE_24:
        *rightLowCut.get<0>().coefficients = *cutCoeffecients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoeffecients[1];
        rightLowCut.setBypassed<1>(false);
        break;

    case SLOPE_36:
        *rightLowCut.get<0>().coefficients = *cutCoeffecients[0];
       rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoeffecients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoeffecients[2];
        rightLowCut.setBypassed<2>(false);
        break;

    case SLOPE_48:
        *rightLowCut.get<0>().coefficients = *cutCoeffecients[0];
        rightLowCut.setBypassed<0>(false);
        *rightLowCut.get<1>().coefficients = *cutCoeffecients[1];
        rightLowCut.setBypassed<1>(false);
        *rightLowCut.get<2>().coefficients = *cutCoeffecients[2];
        rightLowCut.setBypassed<2>(false);
        *rightLowCut.get<3>().coefficients = *cutCoeffecients[3];
        rightLowCut.setBypassed<3>(false);
        break;
    }



    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    juce::dsp::AudioBlock<float> block(buffer);

    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);

    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);

    leftChain.process(leftContext);
    rightChain.process(rightContext);

}

//==============================================================================
bool _3BandEQTutorialAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* _3BandEQTutorialAudioProcessor::createEditor()
{
    //return new _3BandEQTutorialAudioProcessorEditor (*this);

    // Temporary GUI for parameter management
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void _3BandEQTutorialAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void _3BandEQTutorialAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}
// implementing chainsetting grab

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& parameterManager)
{
    ChainSettings settings;

    settings.lowCutFreq = parameterManager.getRawParameterValue("LowCut Freq")->load();
    settings.highCutFreq = parameterManager.getRawParameterValue("HiCut Freq")->load();
    settings.peakFreq = parameterManager.getRawParameterValue("Peak Freq")->load();
    settings.peakGainInDecibels = parameterManager.getRawParameterValue("Peak Gain")->load();
    settings.peakQuality = parameterManager.getRawParameterValue("Quality")->load();
    settings.lowCutSlope = static_cast<Slope>(parameterManager.getRawParameterValue("LowCut Slope")->load());
    settings.highCutSlope = static_cast<Slope>(parameterManager.getRawParameterValue("HiCut Slope")->load());
    return settings; 
}


juce::AudioProcessorValueTreeState::ParameterLayout _3BandEQTutorialAudioProcessor::returnParameterLayout()
{
    // Parameter declaration
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowCut Freq", "LowCut Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 
        20.0f)); // NOTE: For organization purposes, the default set is the bottom number here.
    layout.add(std::make_unique<juce::AudioParameterFloat>("HiCut Freq", "HiCut Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 
        20000.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Freq", "Peak Freq",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.25f), 
        750.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Peak Gain", "Peak Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f, 0.25f),
        0.0f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("Quality", "Quality",
        juce::NormalisableRange<float>(0.10f, 10.0f, 0.05f, 0.25f),
        1.0f));
    // Choice parameter
    juce::StringArray choices;
    for (auto i = 0; i < 4; ++i)
    {
            juce::String tmp;
            tmp << (12 + i * 12);
            tmp << " db/Oct";
            choices.add(tmp);
    }

        layout.add(std::make_unique<juce::AudioParameterChoice>("LowCut Slope", "LowCut Slope", choices, 0));
        layout.add(std::make_unique<juce::AudioParameterChoice>("HiCut Slope", "HiCut Slope", choices, 0));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new _3BandEQTutorialAudioProcessor();
}
