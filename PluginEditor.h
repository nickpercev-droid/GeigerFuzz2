#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class HellfurnaceLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HellfurnaceLookAndFeel();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float,
                          juce::Slider&) override;
};

class HellfurnaceFuzzAudioProcessorEditor : public juce::AudioProcessorEditor,
                                             private juce::Timer
{
public:
    explicit HellfurnaceFuzzAudioProcessorEditor(HellfurnaceFuzzAudioProcessor&);
    ~HellfurnaceFuzzAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    using APVTS = juce::AudioProcessorValueTreeState;
    using SliderAttachment = APVTS::SliderAttachment;
    HellfurnaceFuzzAudioProcessor& processor;
    HellfurnaceLookAndFeel lookAndFeel;
    std::array<juce::Slider, 8> knobs;
    std::array<juce::Label, 8> labels;
    std::array<std::unique_ptr<SliderAttachment>, 8> attachments;
    const std::array<const char*, 8> ids { "FUZZ", "CRUSH", "GATE", "BIAS", "TONE", "BODY", "MIX", "OUTPUT" };
    juce::ComboBox modeBox;
    std::unique_ptr<APVTS::ComboBoxAttachment> modeAttachment;
    juce::ToggleButton powerButton { "ON" };
    std::unique_ptr<APVTS::ButtonAttachment> powerAttachment;
    float meter = 0.0f;
    void timerCallback() override;
    void drawSkull(juce::Graphics&, juce::Rectangle<float>);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HellfurnaceFuzzAudioProcessorEditor)
};
