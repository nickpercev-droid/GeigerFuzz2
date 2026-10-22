#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
    float param(GeigerFuzzAudioProcessor::APVTS& apvts, const char* id)
    {
        return apvts.getRawParameterValue(id)->load();
    }

    float clampSample(float x) { return juce::jlimit(-1.0f, 1.0f, x); }

    // A deliberately compact, original nonlinear transfer bank inspired by
    // digital wavetable/logic distortion; it is not a model of the original pedal.
    float digitalTable(float x, int mode)
    {
        x = clampSample(x);
        switch (mode)
        {
            case 0: // asymmetric hard fold
                return clampSample(std::tanh(5.5f * x) * 1.15f
                    + 0.22f * std::sin(x * 23.0f));
            case 1: // stepped, crunchy transfer
                return std::round(x * 9.0f) / 9.0f;
            case 2: // foldback
            {
                float y = std::fmod(x * 4.0f + 1.0f, 4.0f);
                if (y < 0.0f) y += 4.0f;
                y = y < 2.0f ? y : 4.0f - y;
                return y - 1.0f;
            }
            default: // dense, bit-like nonlinear table
                return clampSample(std::sin((x * 2.8f + x * x * 1.7f) * juce::MathConstants<float>::pi)
                                   * (0.7f + 0.3f * std::abs(x)));
        }
    }
}

GeigerFuzzAudioProcessor::GeigerFuzzAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout()),
      heldValues(2, 0.0f)
{
}

GeigerFuzzAudioProcessor::APVTS::ParameterLayout
GeigerFuzzAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    p.add(std::make_unique<juce::AudioParameterFloat>("drive", "Drive",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 0.72f));
    p.add(std::make_unique<juce::AudioParameterFloat>("bits", "Bit Depth",
        juce::NormalisableRange<float>(1.0f, 16.0f, 1.0f), 5.0f));
    p.add(std::make_unique<juce::AudioParameterFloat>("rate", "Rate Reduce",
        juce::NormalisableRange<float>(1.0f, 32.0f, 1.0f), 8.0f));
    p.add(std::make_unique<juce::AudioParameterChoice>("table", "Table",
        juce::StringArray { "Hard Fold", "Stepped", "Foldback", "Digital Sine" }, 2));
    p.add(std::make_unique<juce::AudioParameterFloat>("tone", "Tone",
        juce::NormalisableRange<float>(500.0f, 18000.0f, 1.0f, 0.35f), 8500.0f));
    p.add(std::make_unique<juce::AudioParameterFloat>("mix", "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.001f), 1.0f));
    p.add(std::make_unique<juce::AudioParameterFloat>("output", "Output",
        juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), -3.0f));
    return p;
}

void GeigerFuzzAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    holdCounter = 0;
    heldValues.assign(static_cast<size_t>(getTotalNumOutputChannels()), 0.0f);
    auto coeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(
        sampleRate, juce::jlimit(100.0, sampleRate * 0.45, 8500.0));
    for (auto& f : toneFilters)
    {
        f.reset();
        f.coefficients = coeff;
    }
}

bool GeigerFuzzAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono()
                      || out == juce::AudioChannelSet::stereo());
}

void GeigerFuzzAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();
    const float drive = param(parameters, "drive");
    const int bits = juce::roundToInt(param(parameters, "bits"));
    const int rate = juce::roundToInt(param(parameters, "rate"));
    const int table = juce::roundToInt(param(parameters, "table"));
    const float mix = param(parameters, "mix");
    const float outGain = juce::Decibels::decibelsToGain(param(parameters, "output"));
    const float tone = param(parameters, "tone");

    auto coeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(
        currentSampleRate, juce::jlimit(100.0, currentSampleRate * 0.45, static_cast<double>(tone)));
    for (auto& f : toneFilters)
        f.coefficients = coeff;

    const float quantLevels = std::pow(2.0f, static_cast<float>(bits - 1));
    const float preGain = 1.0f + drive * 24.0f;

    // Process sample frames together so the sample-and-hold clock is shared
    // across channels and remains stable in stereo.
    for (int i = 0; i < samples; ++i)
    {
        if (holdCounter == 0)
        {
            for (int ch = 0; ch < channels; ++ch)
            {
                const float dry = buffer.getSample(ch, i);
                const float pushed = std::tanh(dry * preGain);
                float shaped = digitalTable(pushed, table);
                shaped = std::round(shaped * quantLevels) / quantLevels;
                heldValues[static_cast<size_t>(juce::jmin(ch, 1))] = shaped;
            }
        }

        for (int ch = 0; ch < channels; ++ch)
        {
            const float dry = buffer.getSample(ch, i);
            const float held = heldValues[static_cast<size_t>(juce::jmin(ch, 1))];
            const float wet = toneFilters[juce::jmin(ch, 1)].processSample(held);
            buffer.setSample(ch, i, ((dry * (1.0f - mix)) + (wet * mix)) * outGain);
        }
        holdCounter = (holdCounter + 1) % rate;
    }
}

juce::AudioProcessorEditor* GeigerFuzzAudioProcessor::createEditor()
{
    return new GeigerFuzzAudioProcessorEditor(*this);
}

void GeigerFuzzAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void GeigerFuzzAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml != nullptr && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GeigerFuzzAudioProcessor();
}
