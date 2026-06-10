/*
  CrystalWaterGoli - VST3 Plugin
  PluginEditor.h
*/
#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class Clystal_Water_GoliAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit Clystal_Water_GoliAudioProcessorEditor (Clystal_Water_GoliAudioProcessor&);
    ~Clystal_Water_GoliAudioProcessorEditor() override;

    void paint   (juce::Graphics&) override;
    void resized () override;

private:
    Clystal_Water_GoliAudioProcessor& audioProcessor;

    juce::TextButton sampleBtns[3];

    juce::Slider thresholdSlider;
    juce::Slider releaseSlider;
    juce::Slider reverbMixSlider;
    juce::Slider dryWetSlider;

    juce::Label  thresholdLabel;
    juce::Label  releaseLabel;
    juce::Label  reverbMixLabel;
    juce::Label  dryWetLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAtt;

    void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name);
    void updateSampleButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Clystal_Water_GoliAudioProcessorEditor)
};
