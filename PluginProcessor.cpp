/*
  CrystalWaterGoli - VST3 Plugin
  PluginProcessor.cpp
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "BinaryData.h"

juce::AudioProcessorValueTreeState::ParameterLayout
Clystal_Water_GoliAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "threshold", 1 }, "Threshold",
        juce::NormalisableRange<float>(0.001f, 0.5f, 0.001f, 0.3f), 0.05f));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "release", 1 }, "Release",
        juce::NormalisableRange<float>(0.05f, 3.0f, 0.001f, 0.4f), 0.5f));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "reverbMix", 1 }, "Reverb",
        0.0f, 1.0f, 0.2f));

    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ "dryWet", 1 }, "Dry/Wet",
        0.0f, 1.0f, 0.9f));

    return layout;
}

Clystal_Water_GoliAudioProcessor::Clystal_Water_GoliAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    loadSample (0, BinaryData::clystal1_wav,  BinaryData::clystal1_wavSize);
    loadSample (1, BinaryData::clystal2_wav,  BinaryData::clystal2_wavSize);
    loadSample (2, BinaryData::water1_wav,    BinaryData::water1_wavSize);
}

Clystal_Water_GoliAudioProcessor::~Clystal_Water_GoliAudioProcessor() {}

void Clystal_Water_GoliAudioProcessor::loadSample (int idx, const char* data, int size)
{
    auto* mis = new juce::MemoryInputStream (data, (size_t)size, false);
    juce::WavAudioFormat fmt;
    std::unique_ptr<juce::AudioFormatReader> reader (fmt.createReaderFor (mis, true));

    if (reader != nullptr)
    {
        sampleBufs[idx].setSize ((int)reader->numChannels,
                                 (int)reader->lengthInSamples);
        reader->read (&sampleBufs[idx], 0,
                      (int)reader->lengthInSamples, 0, true, true);
    }
}

void Clystal_Water_GoliAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sampleRate_ = sampleRate;

    for (int ch = 0; ch < 2; ch++)
    {
        voice[ch]       = {};
        envFollow[ch]   = 0.0f;
        aboveThresh[ch] = false;
    }

    reverb.reset();
    juce::Reverb::Parameters rp;
    rp.roomSize   = 0.45f;
    rp.damping    = 0.65f;
    rp.wetLevel   = 0.15f;
    rp.dryLevel   = 0.85f;
    rp.width      = 0.8f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
}

void Clystal_Water_GoliAudioProcessor::releaseResources() {}

bool Clystal_Water_GoliAudioProcessor::isBusesLayoutSupported (
    const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void Clystal_Water_GoliAudioProcessor::processBlock (
    juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const float threshold   = apvts.getRawParameterValue ("threshold")->load();
    const float releaseTime = apvts.getRawParameterValue ("release")  ->load();
    const float reverbMix   = apvts.getRawParameterValue ("reverbMix")->load();
    const float dryWet      = apvts.getRawParameterValue ("dryWet")   ->load();

    const int sel     = selectedSample.load();
    const int numCh   = buffer.getNumChannels();
    const int numSamp = buffer.getNumSamples();

    const float relCoeff = std::expf (-1.0f / (float)(sampleRate_ * releaseTime));
    const float attCoeff = std::expf (-1.0f / (float)(sampleRate_ * 0.0005));
    const float decCoeff = std::expf (-1.0f / (float)(sampleRate_ * 0.030));

    auto& smpBuf     = sampleBufs[sel];
    const int smpLen = smpBuf.getNumSamples();
    const int smpCh  = smpBuf.getNumChannels();

    for (int ch = 0; ch < std::min (numCh, 2); ch++)
    {
        auto* data = buffer.getWritePointer (ch);

        const float* smpData = (smpLen > 0 && smpCh > 0)
            ? smpBuf.getReadPointer (ch < smpCh ? ch : 0)
            : nullptr;

        for (int i = 0; i < numSamp; i++)
        {
            const float in    = data[i];
            const float absIn = std::abs (in);

            // Envelope follower
            if (absIn > envFollow[ch])
                envFollow[ch] = attCoeff * envFollow[ch] + (1.0f - attCoeff) * absIn;
            else
                envFollow[ch] = decCoeff * envFollow[ch];

            // Attack detection: rising edge crossing threshold
            const bool nowAbove  = (envFollow[ch] >= threshold);
            const bool triggered = (nowAbove && !aboveThresh[ch]);
            aboveThresh[ch] = nowAbove;

            if (triggered)
            {
                voice[ch].active = true;
                voice[ch].pos    = 0;
                voice[ch].env    = 1.0f;
            }

            // Sample playback
            float smpOut = 0.0f;
            if (voice[ch].active && smpData != nullptr && voice[ch].pos < smpLen)
            {
                smpOut = smpData[voice[ch].pos] * voice[ch].env;
                voice[ch].pos++;
                voice[ch].env *= relCoeff;

                if (voice[ch].pos >= smpLen || voice[ch].env < 1e-5f)
                    voice[ch].active = false;
            }

            // Sustain = 0: pass input only while above threshold (attack portion)
            const float gatedIn = nowAbove ? in : 0.0f;

            // Dry/Wet mix
            data[i] = gatedIn * (1.0f - dryWet) + smpOut * dryWet;
        }
    }

    // Reverb
    if (numCh >= 2)
    {
        juce::Reverb::Parameters rp;
        rp.roomSize   = 0.45f;
        rp.damping    = 0.65f;
        rp.wetLevel   = reverbMix * 0.45f;
        rp.dryLevel   = 1.0f - reverbMix * 0.15f;
        rp.width      = 0.85f;
        rp.freezeMode = 0.0f;
        reverb.setParameters (rp);

        reverb.processStereo (buffer.getWritePointer (0),
                              buffer.getWritePointer (1),
                              numSamp);
    }
}

juce::AudioProcessorEditor* Clystal_Water_GoliAudioProcessor::createEditor()
{
    return new Clystal_Water_GoliAudioProcessorEditor (*this);
}

bool               Clystal_Water_GoliAudioProcessor::hasEditor()            const { return true; }
const juce::String Clystal_Water_GoliAudioProcessor::getName()              const { return JucePlugin_Name; }
bool               Clystal_Water_GoliAudioProcessor::acceptsMidi()          const { return false; }
bool               Clystal_Water_GoliAudioProcessor::producesMidi()         const { return false; }
bool               Clystal_Water_GoliAudioProcessor::isMidiEffect()         const { return false; }
double             Clystal_Water_GoliAudioProcessor::getTailLengthSeconds() const { return 3.0; }

int  Clystal_Water_GoliAudioProcessor::getNumPrograms()                          { return 1; }
int  Clystal_Water_GoliAudioProcessor::getCurrentProgram()                       { return 0; }
void Clystal_Water_GoliAudioProcessor::setCurrentProgram (int)                   {}
const juce::String Clystal_Water_GoliAudioProcessor::getProgramName (int)        { return {}; }
void Clystal_Water_GoliAudioProcessor::changeProgramName (int, const juce::String&) {}

void Clystal_Water_GoliAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void Clystal_Water_GoliAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Clystal_Water_GoliAudioProcessor();
}
