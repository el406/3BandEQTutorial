/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


// We save temp parameters as a struct so its easy to access
enum Slope
{
    SLOPE_12,
    SLOPE_24,
    SLOPE_36,
    SLOPE_48
};

struct ChainSettings
{
    float peakFreq{ 0 }, peakGainInDecibels{ 0 }, peakQuality{ 1.f };
    float lowCutFreq{ 0 }, highCutFreq{ 0 };
    Slope lowCutSlope{ Slope::SLOPE_12 }, highCutSlope{ Slope::SLOPE_12 };

};


ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& parameterManager);


//==============================================================================
/**
*/
class _3BandEQTutorialAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    _3BandEQTutorialAudioProcessor();
    ~_3BandEQTutorialAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout returnParameterLayout();

    // Declaration of parameter variable
    juce::AudioProcessorValueTreeState parameterManager{ *this, nullptr, "Parameters", returnParameterLayout()};

private:
    // Template definitions
    using Filter = juce::dsp::IIR::Filter<float>;

    using CutFilter = juce::dsp::ProcessorChain < Filter, Filter, Filter, Filter>;

    using MonoChain = juce::dsp::ProcessorChain <CutFilter, Filter, CutFilter>;

    MonoChain leftChain, rightChain;

    enum ChainPositions
    {
        LowCut,
        Peak,
        HighCut
    };

    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (_3BandEQTutorialAudioProcessor)
};
