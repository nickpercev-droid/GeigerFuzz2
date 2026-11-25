#pragma once
#include <JuceHeader.h>

class HellfurnaceFuzzAudioProcessor : public juce::AudioProcessor
{
public:
    HellfurnaceFuzzAudioProcessor();
    ~HellfurnaceFuzzAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "HELLFURNACE FUZZ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    float getMeterLevel() const noexcept { return meterLevel.load(); }
    static APVTS::ParameterLayout createParameterLayout();

private:
    std::atomic<float> meterLevel { 0.0f };
    float gateEnvelope[2] { 0.0f, 0.0f };
    float dcState[2] { 0.0f, 0.0f };
    double sr = 44100.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HellfurnaceFuzzAudioProcessor)
};
