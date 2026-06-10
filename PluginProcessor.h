/*
  CrystalWaterGoli - VST3 Plugin
  PluginProcessor.h
*/
#pragma once
#include <JuceHeader.h>

class Clystal_Water_GoliAudioProcessor : public juce::AudioProcessor
{
public:
    Clystal_Water_GoliAudioProcessor();
    ~Clystal_Water_GoliAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi()  const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int  getNumPrograms() override;
    int  getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<int> selectedSample { 0 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void loadSample (int idx, const char* data, int size);
    juce::AudioBuffer<float> sampleBufs[3];

    struct Voice {
        int   pos    = 0;
        bool  active = false;
        float env    = 0.0f;
    };
    Voice voice[2];

    float envFollow[2]    = { 0.0f, 0.0f };
    bool  aboveThresh[2]  = { false, false };

    juce::Reverb reverb;
    double sampleRate_ = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Clystal_Water_GoliAudioProcessor)
};
