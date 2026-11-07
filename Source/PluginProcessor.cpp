#include "PluginProcessor.h"
#include "PluginEditor.h"

BassRigProcessor::BassRigProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    pAmp    = apvts.getRawParameterValue ("amp");
    pCab    = apvts.getRawParameterValue ("cab");
    pMic    = apvts.getRawParameterValue ("mic");
    pPos    = apvts.getRawParameterValue ("pos");
    pDist   = apvts.getRawParameterValue ("distance");
    pGain   = apvts.getRawParameterValue ("gain");
    pBass   = apvts.getRawParameterValue ("bass");
    pMid    = apvts.getRawParameterValue ("mid");
    pTreble = apvts.getRawParameterValue ("treble");
    pVolume = apvts.getRawParameterValue ("volume");
}

juce::AudioProcessorValueTreeState::ParameterLayout BassRigProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "amp", 1 }, "Amp",
                StringArray { "Markbass Little Mark 3", "Ampeg SVT", "Hartke LX8500" }, 1));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "cab", 1 }, "Cab",
                StringArray { "1x15", "4x10", "8x10" }, 1));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "mic", 1 }, "Mic",
                StringArray { "AEA R84", "AKG C414 XLS", "Heil PR40" }, 2));
    layout.add (std::make_unique<AudioParameterChoice> (ParameterID { "pos", 1 }, "Mic Position",
                StringArray { "Centro", "Meio", "Cone" }, 1));
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { "distance", 1 }, "Mic Distance (cm)",
                NormalisableRange<float> (1.0f, 100.0f, 0.1f, 0.6f), 15.0f));

    auto knob = [&layout] (const char* id, const char* name, float def)
    {
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name,
                    NormalisableRange<float> (0.0f, 10.0f, 0.01f), def));
    };
    knob ("gain",   "Gain",   4.0f);
    knob ("bass",   "Bass",   5.0f);
    knob ("mid",    "Mid",    5.0f);
    knob ("treble", "Treble", 5.0f);
    knob ("volume", "Volume", 5.0f);

    return layout;
}

void BassRigProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    maxBlock = juce::jmax (64, samplesPerBlock);
    mono.assign ((size_t) maxBlock, 0.0f);
    outL.assign ((size_t) maxBlock, 0.0f);
    outR.assign ((size_t) maxBlock, 0.0f);

    engine.prepare (sampleRate, maxBlock);
    setLatencySamples (engine.getLatency());
}

bool BassRigProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in  = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    const bool inOk  = (in  == juce::AudioChannelSet::mono() || in  == juce::AudioChannelSet::stereo());
    const bool outOk = (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
    return inOk && outOk;
}

void BassRigProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numIn  = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int total  = buffer.getNumSamples();

    bassdsp::BassEngine::Settings s;
    s.amp        = (int) std::lround (pAmp->load());
    s.cab        = (int) std::lround (pCab->load());
    s.mic        = (int) std::lround (pMic->load());
    s.pos        = (int) std::lround (pPos->load());
    s.distanceCm = pDist->load();
    s.gain       = pGain->load();
    s.bass       = pBass->load();
    s.mid        = pMid->load();
    s.treble     = pTreble->load();
    s.volume     = pVolume->load();

    int start = 0;
    while (start < total)
    {
        const int n = juce::jmin (total - start, maxBlock);

        // Baixo e mono: soma os canais de entrada
        for (int i = 0; i < n; ++i) mono[(size_t) i] = 0.0f;
        for (int ch = 0; ch < numIn; ++ch)
        {
            const float* src = buffer.getReadPointer (ch) + start;
            for (int i = 0; i < n; ++i) mono[(size_t) i] += src[i];
        }
        if (numIn > 1)
            for (int i = 0; i < n; ++i) mono[(size_t) i] /= (float) numIn;

        engine.process (mono.data(), outL.data(), outR.data(), n, s);

        if (numOut == 1)
        {
            float* d = buffer.getWritePointer (0) + start;
            for (int i = 0; i < n; ++i) d[i] = 0.5f * (outL[(size_t) i] + outR[(size_t) i]);
        }
        else
        {
            for (int ch = 0; ch < numOut; ++ch)
            {
                float* d = buffer.getWritePointer (ch) + start;
                const auto& src = (ch == 0) ? outL : outR;
                for (int i = 0; i < n; ++i) d[i] = src[(size_t) i];
            }
        }
        start += n;
    }
}

juce::AudioProcessorEditor* BassRigProcessor::createEditor() { return new BassRigEditor (*this); }

void BassRigProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void BassRigProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BassRigProcessor(); }
