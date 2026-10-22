#include "PluginEditor.h"

GeigerFuzzAudioProcessorEditor::GeigerFuzzAudioProcessorEditor(GeigerFuzzAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(620, 370);
    title.setText("GEIGER FUZZ", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    title.setJustificationType(juce::Justification::centredLeft);
    title.setColour(juce::Label::textColourId, juce::Colour(0xffe9ff5a));
    addAndMakeVisible(title);

    subtitle.setText("DIGITAL WAVEFORM DESTROYER  /  VST3", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(11.0f)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffb3b99a));
    addAndMakeVisible(subtitle);

    juce::Slider* sliders[] = { &drive, &bits, &rate, &table, &tone, &mix, &output };
    const char* ids[] = { "drive", "bits", "rate", "table", "tone", "mix", "output" };
    const char* names[] = { "DRIVE", "BIT DEPTH", "RATE REDUCE", "WAVE TABLE", "TONE", "MIX", "OUTPUT" };
    for (int i = 0; i < 7; ++i)
    {
        auto* s = sliders[i];
        s->setSliderStyle(i == 3 ? juce::Slider::RotaryHorizontalVerticalDrag : juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 78, 20);
        s->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffe9ff5a));
        s->setColour(juce::Slider::thumbColourId, juce::Colour(0xffe9ff5a));
        s->setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        s->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff252b20));
        s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff566044));
        addAndMakeVisible(*s);
        labels[i].setText(names[i], juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffe0e5d6));
        addAndMakeVisible(labels[i]);
        attachments[i] = std::make_unique<SliderAttachment>(processor.parameters, ids[i], *s);
    }
    table.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 92, 20);
}

void GeigerFuzzAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff141812));
    g.setColour(juce::Colour(0xff2b3323));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(10.0f), 12.0f);
    g.setColour(juce::Colour(0xffe9ff5a));
    g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(10.0f), 12.0f, 1.5f);
    g.setColour(juce::Colour(0xff77805e));
    g.drawLine(24.0f, 82.0f, 596.0f, 82.0f, 1.0f);
}

void GeigerFuzzAudioProcessorEditor::resized()
{
    title.setBounds(24, 18, 300, 40);
    subtitle.setBounds(26, 56, 350, 18);
    const int left = 22, top = 95, w = 576, cellW = 144, cellH = 130;
    juce::Slider* sliders[] = { &drive, &bits, &rate, &table, &tone, &mix, &output };
    for (int i = 0; i < 7; ++i)
    {
        const int row = i / 4;
        const int col = i % 4;
        const int x = left + col * cellW;
        const int y = top + row * cellH;
        labels[i].setBounds(x, y, cellW - 4, 18);
        sliders[i]->setBounds(x + 8, y + 16, cellW - 20, 108);
    }
}
