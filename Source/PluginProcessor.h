#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP.h"

class BassRigProcessor : public juce::AudioProcessor
{
public:
    BassRigProcessor();
    ~BassRigProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "BassRig"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 1.5; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::AudioProcessorValueTreeState apvts;

private:
    bassdsp::BassEngine engine;
    std::vector<float> mono, outL, outR;
    int maxBlock = 512;

    std::atomic<float>* pAmp = nullptr;
    std::atomic<float>* pCab = nullptr;
    std::atomic<float>* pMic = nullptr;
    std::atomic<float>* pPos = nullptr;
    std::atomic<float>* pDist = nullptr;
    std::atomic<float>* pGain = nullptr;
    std::atomic<float>* pBass = nullptr;
    std::atomic<float>* pMid = nullptr;
    std::atomic<float>* pTreble = nullptr;
    std::atomic<float>* pVolume = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BassRigProcessor)
};
