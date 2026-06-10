/*
  CrystalWaterGoli - VST3 Plugin
  PluginEditor.cpp
*/
#include "PluginProcessor.h"
#include "PluginEditor.h"

Clystal_Water_GoliAudioProcessorEditor::Clystal_Water_GoliAudioProcessorEditor (
    Clystal_Water_GoliAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (460, 280);

    const char* btnNames[] = { "clystal-1", "clystal-2", "water-1" };
    for (int i = 0; i < 3; i++)
    {
        sampleBtns[i].setButtonText (btnNames[i]);
        sampleBtns[i].onClick = [this, i]
        {
            audioProcessor.selectedSample.store (i);
            updateSampleButtons();
        };
        addAndMakeVisible (sampleBtns[i]);
    }

    setupKnob (thresholdSlider, thresholdLabel, "Threshold");
    setupKnob (releaseSlider,   releaseLabel,   "Release");
    setupKnob (reverbMixSlider, reverbMixLabel, "Reverb");
    setupKnob (dryWetSlider,    dryWetLabel,    "Dry/Wet");

    auto& apvts = audioProcessor.apvts;
    thresholdAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (apvts, "threshold", thresholdSlider);
    releaseAtt   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (apvts, "release", releaseSlider);
    reverbMixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (apvts, "reverbMix", reverbMixSlider);
    dryWetAtt    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
        (apvts, "dryWet", dryWetSlider);

    updateSampleButtons();
}

Clystal_Water_GoliAudioProcessorEditor::~Clystal_Water_GoliAudioProcessorEditor() {}

void Clystal_Water_GoliAudioProcessorEditor::setupKnob (
    juce::Slider& s, juce::Label& l, const juce::String& name)
{
    s.setSliderStyle  (juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 16);
    addAndMakeVisible (s);

    l.setText              (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont              (juce::FontOptions (11.0f));
    l.setColour            (juce::Label::textColourId, juce::Colour (0xffaabbcc));
    addAndMakeVisible      (l);
}

void Clystal_Water_GoliAudioProcessorEditor::updateSampleButtons()
{
    const int sel = audioProcessor.selectedSample.load();
    const juce::Colour activeCol   (0xff55ccff);
    const juce::Colour inactiveCol (0xff2a3a4a);
    const juce::Colour activeText  (0xff001122);
    const juce::Colour inactiveText(0xff88aabb);

    for (int i = 0; i < 3; i++)
    {
        sampleBtns[i].setColour (juce::TextButton::buttonColourId,
                                 i == sel ? activeCol    : inactiveCol);
        sampleBtns[i].setColour (juce::TextButton::textColourOffId,
                                 i == sel ? activeText   : inactiveText);
    }
}

void Clystal_Water_GoliAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff16202e));

    g.setColour (juce::Colour (0xff88ddff));
    g.setFont   (juce::FontOptions (20.0f));
    g.drawText  ("Crystal Water Goli", 0, 8, getWidth(), 26,
                 juce::Justification::centred);

    g.setColour (juce::Colour (0xff445566));
    g.setFont   (juce::FontOptions (10.0f));
    g.drawText  ("SAMPLE SELECT", 20, 48, getWidth() - 40, 12,
                 juce::Justification::centred);
    g.drawText  ("PARAMETERS", 20, 140, getWidth() - 40, 12,
                 juce::Justification::centred);

    g.setColour (juce::Colour (0xff2a3a4a));
    g.drawHorizontalLine (44,  20.0f, (float)(getWidth() - 20));
    g.drawHorizontalLine (136, 20.0f, (float)(getWidth() - 20));
}

void Clystal_Water_GoliAudioProcessorEditor::resized()
{
    const int btnW = 120, btnH = 28, btnY = 62;
    const int btnTotalW = 3 * btnW + 2 * 10;
    const int btnStartX = (getWidth() - btnTotalW) / 2;
    for (int i = 0; i < 3; i++)
        sampleBtns[i].setBounds (btnStartX + i * (btnW + 10), btnY, btnW, btnH);

    const int knobSz  = 70;
    const int labelH  = 18;
    const int knobY   = 156;
    const int spacing = 94;
    const int startX  = (getWidth() - spacing * 3 - knobSz) / 2;

    thresholdSlider.setBounds (startX + spacing * 0, knobY, knobSz, knobSz);
    thresholdLabel .setBounds (startX + spacing * 0, knobY + knobSz, knobSz, labelH);
    releaseSlider  .setBounds (startX + spacing * 1, knobY, knobSz, knobSz);
    releaseLabel   .setBounds (startX + spacing * 1, knobY + knobSz, knobSz, labelH);
    reverbMixSlider.setBounds (startX + spacing * 2, knobY, knobSz, knobSz);
    reverbMixLabel .setBounds (startX + spacing * 2, knobY + knobSz, knobSz, labelH);
    dryWetSlider   .setBounds (startX + spacing * 3, knobY, knobSz, knobSz);
    dryWetLabel    .setBounds (startX + spacing * 3, knobY + knobSz, knobSz, labelH);
}
