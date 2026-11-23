#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class GeigerCounterProAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit GeigerCounterProAudioProcessorEditor(GeigerCounterProAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    GeigerCounterProAudioProcessor& processor;
    juce::Label title, subtitle, display;
    std::array<juce::Slider, 11> knobs;
    std::array<juce::Label, 11> names;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,11> attachments;
    std::array<juce::ToggleButton, 7> toggles;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>,7> toggleAttachments;
    juce::TextButton randomButton { "RAND" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeigerCounterProAudioProcessorEditor)
};
