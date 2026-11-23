#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>

class GeigerCounterProAudioProcessor final : public juce::AudioProcessor
{
public:
    GeigerCounterProAudioProcessor();
    ~GeigerCounterProAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 16; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    static APVTS::ParameterLayout createParameterLayout();
    void randomizeParameters();

private:
    static constexpr int tableSize = 2048;
    static constexpr int tableCount = 512;
    using WaveTable = std::array<float, tableSize>;
    std::array<WaveTable, tableCount> waveTables{};
    void buildWaveTables();
    float tableLookup(float x, int bank, int table, float morph) const noexcept;
    float read(const char* id) const noexcept;

    double sampleRateHz = 44100.0;
    int holdCounter = 0;
    int holdPeriod = 1;
    std::array<float, 2> held{};
    std::array<float, 2> previous{};
    std::array<juce::dsp::IIR::Filter<float>, 2> toneFilters;
    std::array<juce::dsp::IIR::Filter<float>, 2> postFilters;
    std::array<float, 16> programSeeds{};
    int currentProgram = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GeigerCounterProAudioProcessor)
};
