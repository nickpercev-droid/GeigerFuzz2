#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

HellfurnaceFuzzAudioProcessor::HellfurnaceFuzzAudioProcessor()
: AudioProcessor(BusesProperties()
    .withInput("Input", juce::AudioChannelSet::stereo(), true)
    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
  parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{}

HellfurnaceFuzzAudioProcessor::APVTS::ParameterLayout
HellfurnaceFuzzAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    auto add = [&p](const char* id, const char* name, float lo, float hi, float def,
                    juce::AudioParameterFloatAttributes attr = {})
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID{id, 1}, name,
            juce::NormalisableRange<float>(lo, hi), def, attr));
    };
    add("FUZZ", "FUZZ", 0.0f, 1.0f, 0.92f);
    add("CRUSH", "CRUSH", 0.0f, 1.0f, 0.35f);
    add("GATE", "GATE", 0.0f, 1.0f, 0.18f);
    add("BIAS", "BIAS", -1.0f, 1.0f, 0.25f);
    add("TONE", "TONE", 0.0f, 1.0f, 0.58f);
    add("BODY", "BODY", 0.0f, 1.0f, 0.72f);
    add("MIX", "MIX", 0.0f, 1.0f, 1.0f);
    add("OUTPUT", "OUTPUT", 0.0f, 1.0f, 0.72f);
    p.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{"MODE", 1}, "MODE",
        juce::StringArray{"Classic", "Searing", "Chaos Max"}, 2));
    p.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"ON", 1}, "ON", true));
    return p;
}

void HellfurnaceFuzzAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = sampleRate;
    gateEnvelope[0] = gateEnvelope[1] = 0.0f;
    dcState[0] = dcState[1] = 0.0f;
}

bool HellfurnaceFuzzAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet();
    const auto out = l.getMainOutputChannelSet();
    return (in == juce::AudioChannelSet::mono() || in == juce::AudioChannelSet::stereo())
        && in == out;
}

void HellfurnaceFuzzAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = buffer.getNumChannels();
    const int samples = buffer.getNumSamples();

    const float fuzz = parameters.getRawParameterValue("FUZZ")->load();
    const float crush = parameters.getRawParameterValue("CRUSH")->load();
    const float gate = parameters.getRawParameterValue("GATE")->load();
    const float bias = parameters.getRawParameterValue("BIAS")->load();
    const float tone = parameters.getRawParameterValue("TONE")->load();
    const float body = parameters.getRawParameterValue("BODY")->load();
    const float mix = parameters.getRawParameterValue("MIX")->load();
    const float output = parameters.getRawParameterValue("OUTPUT")->load();
    const int mode = int(parameters.getRawParameterValue("MODE")->load());
    const bool enabled = parameters.getRawParameterValue("ON")->load() > 0.5f;
    const float modeDrive = mode == 0 ? 0.72f : (mode == 1 ? 1.18f : 1.65f);

    const float drive = 1.0f + fuzz * 95.0f * modeDrive;
    const float quantSteps = 256.0f - crush * 252.0f;
    const float threshold = 0.0005f + gate * 0.075f;
    const float gateRelease = std::exp(-1.0f / (float(sr) * (0.004f + (1.0f-gate)*0.18f)));
    const float lowGain = 0.45f + body * 1.8f;
    const float highBlend = 0.12f + tone * 0.88f;
    const float outGain = output * output * 1.8f;

    float peak = 0.0f;\n    for (int ch = 0; ch < channels; ++ch)
    {
        auto* x = buffer.getWritePointer(ch);
        const int stateCh = ch < 2 ? ch : 1;
        float env = gateEnvelope[stateCh];
        float dc = dcState[stateCh];
        float low = 0.0f;
        float held = 0.0f;
        int holdCounter = 0;
        const int holdPeriod = 1 + int(crush * 23.0f);

        for (int i = 0; i < samples; ++i)
        {
            const float dry = x[i];
            const float rect = std::abs(dry);
            env = std::max(rect, env * gateRelease);
            const float gateGain = gate <= 0.001f ? 1.0f :
                juce::jlimit(0.0f, 1.0f, (env - threshold * 0.35f) / (threshold * 1.8f));

            // Input stage: asymmetric transistor-like clipping.
            float v = dry * drive + bias * fuzz * 2.2f;
            v = std::tanh(v * (1.0f + fuzz * 2.0f));
            v = v - bias * fuzz * 0.22f;
            v = std::tanh(v * (1.0f + fuzz * 5.0f));

            // Deliberately savage quantisation and sample reduction.
            if (++holdCounter >= holdPeriod)
            {
                held = std::round(v * quantSteps) / std::max(4.0f, quantSteps);
                holdCounter = 0;
            }
            v = v * (1.0f - crush) + held * crush;

            // Low-frequency body + one-pole tone shaping.
            low += 0.035f * (v - low);
            const float shaped = low * lowGain + (v - low) * highBlend;
            v = std::tanh(shaped * (2.0f + fuzz * 7.0f));

            // DC blocking and hard gate.
            const float blocked = v - dc + 0.995f * dcState[stateCh];
            dc = v;
            v = blocked * gateGain;

            const float wet = v * outGain;
            x[i] = dry * (1.0f - mix) + wet * mix;
        }
        gateEnvelope[stateCh] = env;
        dcState[stateCh] = dc;
    }
}

void HellfurnaceFuzzAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void HellfurnaceFuzzAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HellfurnaceFuzzAudioProcessor();
}
