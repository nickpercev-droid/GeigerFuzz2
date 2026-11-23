#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace {
constexpr float pi = juce::MathConstants<float>::pi;
float clamp01(float x) { return juce::jlimit(0.0f, 1.0f, x); }
float quantize12(float x) { return std::round(juce::jlimit(-1.0f, 1.0f, x) * 2047.0f) / 2047.0f; }
}

GeigerCounterProAudioProcessor::GeigerCounterProAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
    buildWaveTables();
}

GeigerCounterProAudioProcessor::APVTS::ParameterLayout GeigerCounterProAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    auto f = [&p](const char* id, const char* name, float lo, float hi, float def, float step = 0.001f) {
        p.add(std::make_unique<juce::AudioParameterFloat>(id, name, juce::NormalisableRange<float>(lo, hi, step), def));
    };
    auto b = [&p](const char* id, const char* name, bool def) { p.add(std::make_unique<juce::AudioParameterBool>(id, name, def)); };
    f("gain", "Gain", 0.0f, 1.0f, 0.45f);
    f("tone", "Tone", 0.0f, 1.0f, 0.65f);
    b("toneOn", "Tone On", true);
    f("mix", "Mix", 0.0f, 1.0f, 1.0f);
    b("dry", "Dry", false);
    f("samples", "Samples", 0.0f, 1.0f, 1.0f);
    b("fine", "Fine", false);
    f("bits", "Bits", 0.0f, 1.0f, 0.72f);
    b("mask", "Mask", false);
    b("preWT", "Pre WT", false);
    f("bank", "Bank", 0.0f, 31.0f, 0.0f, 1.0f);
    f("table", "Table", 0.0f, 15.0f, 0.0f, 1.0f);
    b("morph", "Morph", false);
    f("filter", "Filter", 0.0f, 1.0f, 1.0f);
    f("level", "Level", 0.0f, 1.0f, 0.8f);
    f("master", "Master", 0.0f, 1.0f, 0.8f);
    b("active", "Activate", true);
    return p;
}

float GeigerCounterProAudioProcessor::read(const char* id) const noexcept { return parameters.getRawParameterValue(id)->load(); }

void GeigerCounterProAudioProcessor::buildWaveTables()
{
    // Procedural, original tables. They reproduce the 32 x 16 table architecture,
    // not WMD's proprietary factory wavetable data.
    for (int t = 0; t < tableCount; ++t) {
        const int bank = t / 16, slot = t % 16;
        const float a = 1.0f + (slot % 4) * 0.75f + (bank % 5) * 0.19f;
        const float b = 1.0f + (slot / 4) * 1.7f + (bank % 7) * 0.11f;
        const float fold = 1.2f + (bank % 9) * 0.42f;
        for (int i = 0; i < tableSize; ++i) {
            const float x = (2.0f * i / (tableSize - 1)) - 1.0f;
            float y = 0.0f;
            switch (bank % 8) {
                case 0: y = std::tanh(x * a * 2.0f) * 0.9f + 0.1f * std::sin(x * b * 18.0f); break;
                case 1: y = std::sin((x * a + x * x * 0.7f) * pi * b); break;
                case 2: y = std::round(x * (2.0f + slot)) / (2.0f + slot); break;
                case 3: { float q = std::fmod((x + 1.0f) * fold, 4.0f); if (q < 0) q += 4.0f; y = (q < 2 ? q : 4-q) - 1.0f; break; }
                case 4: y = std::sin(pi * (x * fold + 0.24f * std::sin(x * b * pi))); break;
                case 5: y = std::tanh((x + 0.3f * std::sin(x * b * pi)) * fold); break;
                case 6: y = (x >= 0 ? 1.0f : -1.0f) * std::pow(std::abs(x), 0.25f + 0.08f * slot); break;
                default: y = std::sin(x * pi * a) * std::cos(x * pi * b) * (0.65f + 0.35f * std::abs(x)); break;
            }
            waveTables[t][static_cast<size_t>(i)] = juce::jlimit(-1.0f, 1.0f, y);
        }
    }
}

float GeigerCounterProAudioProcessor::tableLookup(float x, int bank, int table, float morph) const noexcept
{
    x = juce::jlimit(-1.0f, 1.0f, x);
    const float pos = (x + 1.0f) * 0.5f * (tableSize - 1);
    const int i = juce::jlimit(0, tableSize - 2, static_cast<int>(pos));
    const float frac = pos - static_cast<float>(i);
    const int idxA = juce::jlimit(0, tableCount - 1, bank * 16 + table);
    const int idxB = juce::jlimit(0, tableCount - 1, bank * 16 + ((table + 1) & 15));
    auto interp = [&](int idx) { return waveTables[static_cast<size_t>(idx)][static_cast<size_t>(i)] * (1.0f-frac) + waveTables[static_cast<size_t>(idx)][static_cast<size_t>(i+1)] * frac; };
    const float a = interp(idxA);
    return morph > 0.5f ? a * 0.5f + interp(idxB) * 0.5f : a;
}

void GeigerCounterProAudioProcessor::prepareToPlay(double sr, int)
{
    sampleRateHz = sr; holdCounter = 0; held.fill(0.0f); previous.fill(0.0f);
    for (auto& f : toneFilters) f.reset();
    for (auto& f : postFilters) f.reset();
}

bool GeigerCounterProAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const
{
    const auto in = l.getMainInputChannelSet(), out = l.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void GeigerCounterProAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin(2, buffer.getNumChannels());
    const float gain = read("gain"), tone = read("tone"), mix = read("mix");
    const bool toneOn = read("toneOn") > 0.5f, dryMode = read("dry") > 0.5f;
    const bool fine = read("fine") > 0.5f, mask = read("mask") > 0.5f, preWT = read("preWT") > 0.5f;
    const bool morph = read("morph") > 0.5f, active = read("active") > 0.5f;
    const int bank = juce::jlimit(0,31,juce::roundToInt(read("bank")));
    const int table = juce::jlimit(0,15,juce::roundToInt(read("table")));
    const int bits = juce::jlimit(1,12,1 + juce::roundToInt(read("bits") * 11.0f));
    const float sampleNorm = fine ? std::pow(read("samples"), 2.0f) : read("samples");
    const float targetRate = 120.0f * std::pow(static_cast<float>(sampleRateHz) / 120.0f, sampleNorm);
    holdPeriod = juce::jlimit(1, 4096, juce::roundToInt(static_cast<float>(sampleRateHz) / juce::jmax(1.0f, targetRate)));
    const float cutoff = 180.0f * std::pow(110.0f, read("filter"));
    auto toneCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRateHz, juce::jlimit(150.0, sampleRateHz*0.45, static_cast<double>(700.0 + tone * 15000.0)), 0.707);
    auto postCoeff = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRateHz, juce::jlimit(20.0, sampleRateHz*0.45, static_cast<double>(cutoff)), 0.707);
    for (auto& f : toneFilters) f.coefficients = toneCoeff;
    for (auto& f : postFilters) f.coefficients = postCoeff;
    const float preGain = 1.0f + gain * 38.0f;
    const float levels = std::pow(2.0f, static_cast<float>(bits - 1));
    const float level = read("level"), master = read("master");
    const float outGain = level * master * 1.65f;

    for (int i=0; i<buffer.getNumSamples(); ++i) {
        if (holdCounter == 0) {
            for (int ch=0; ch<channels; ++ch) {
                const float input = buffer.getSample(ch,i);
                float pre = std::tanh(input * preGain * 1.35f);
                if (toneOn) pre = toneFilters[static_cast<size_t>(ch)].processSample(pre);
                float processed = pre;
                if (preWT) processed = std::round(processed * levels) / levels;
                if (mask) {
                    const int raw = static_cast<int>((processed + 1.0f) * 2047.5f);
                    const int masked = raw & 0x0FF0;
                    processed = static_cast<float>(masked) / 2047.5f - 1.0f;
                }
                processed = quantize12(processed);
                processed = tableLookup(processed, bank, table, morph ? 1.0f : 0.0f);
                if (!preWT) processed = std::round(processed * levels) / levels;
                held[static_cast<size_t>(ch)] = processed;
            }
        }
        for (int ch=0; ch<channels; ++ch) {
            const float dry = buffer.getSample(ch,i);
            const float analog = std::tanh(dry * preGain * 1.35f);
            float wet = active ? postFilters[static_cast<size_t>(ch)].processSample(held[static_cast<size_t>(ch)]) : dry;
            const float dryPath = dryMode ? dry : analog;
            const float output = active ? (dryPath * (1.0f-mix) + wet * mix) : dry;
            buffer.setSample(ch,i, output * outGain);
        }
        holdCounter = (holdCounter + 1) % holdPeriod;
    }
}

void GeigerCounterProAudioProcessor::randomizeParameters()
{
    juce::Random r;
    for (auto* id : {"gain","tone","mix","samples","bits","bank","table","filter","level"}) {
        auto* p = parameters.getParameter(id);
        p->setValueNotifyingHost(r.nextFloat());
    }
}

void GeigerCounterProAudioProcessor::setCurrentProgram(int index)
{
    currentProgram = juce::jlimit(0,15,index);
    // Compact deterministic starter patches; each program recalls a distinct table region.
    parameters.getParameter("bank")->setValueNotifyingHost(static_cast<float>(currentProgram) / 31.0f);
    parameters.getParameter("table")->setValueNotifyingHost(static_cast<float>((currentProgram*5)%16) / 15.0f);
}
const juce::String GeigerCounterProAudioProcessor::getProgramName(int index) { return "Patch " + juce::String(index+1); }

void GeigerCounterProAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml,dest);
}
void GeigerCounterProAudioProcessor::setStateInformation(const void* data,int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data,size));
    if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* GeigerCounterProAudioProcessor::createEditor() { return new GeigerCounterProAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GeigerCounterProAudioProcessor(); }
