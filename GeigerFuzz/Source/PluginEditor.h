#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class GeigerFuzzAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit GeigerFuzzAudioProcessorEditor(GeigerFuzzAudioProcessor&);
    ~GeigerFuzzAudioProcessorEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    GeigerFuzzAudioProcessor& processor;
    juce::Label title;
    juce::Label subtitle;
    juce::Slider drive, bits, rate, tone, mix, output, table;
    juce::Label labels[7];
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attachments[7];
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeigerFuzzAudioProcessorEditor)
};
