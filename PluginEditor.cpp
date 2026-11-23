#include "PluginEditor.h"

GeigerCounterProAudioProcessorEditor::GeigerCounterProAudioProcessorEditor(GeigerCounterProAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 590);
    title.setText("GEIGER COUNTER PRO", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(28.0f, juce::Font::bold)));
    title.setColour(juce::Label::textColourId, juce::Colour(0xffffd34e));
    addAndMakeVisible(title);
    subtitle.setText("12-BIT WAVETABLE PROCESSOR  /  BEHAVIORAL EDITION", juce::dontSendNotification);
    subtitle.setFont(juce::Font(juce::FontOptions(10.5f)));
    subtitle.setColour(juce::Label::textColourId, juce::Colour(0xffb8b8b0)); addAndMakeVisible(subtitle);
    const char* ids[] = {"gain","tone","mix","filter","samples","bits","bank","table","level","master",""};
    const char* labels[] = {"GAIN","TONE","MIX","FILTER","SAMPLES","BITS","BANK","TABLE","LEVEL","MASTER",""};
    for (int i=0;i<10;++i) {
        auto& s=knobs[static_cast<size_t>(i)]; s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,18);
        s.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffffc83d));
        s.setColour(juce::Slider::thumbColourId,juce::Colour(0xffffdf78));
        s.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
        s.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff252525));
        addAndMakeVisible(s);
        names[static_cast<size_t>(i)].setText(labels[i],juce::dontSendNotification);
        names[static_cast<size_t>(i)].setJustificationType(juce::Justification::centred);
        names[static_cast<size_t>(i)].setFont(juce::Font(juce::FontOptions(10.0f,juce::Font::bold)));
        names[static_cast<size_t>(i)].setColour(juce::Label::textColourId,juce::Colour(0xffe4e1d8)); addAndMakeVisible(names[static_cast<size_t>(i)]);
        attachments[static_cast<size_t>(i)]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters,ids[i],s);
    }
    const char* toggleIds[]={"toneOn","dry","fine","mask","preWT","morph","active"};
    const char* toggleNames[]={"TONE ON","DRY","FINE","MASK","PRE WT","MORPH","ACTIVATE"};
    for(int i=0;i<7;++i){ auto& t=toggles[static_cast<size_t>(i)]; t.setButtonText(toggleNames[i]); t.setColour(juce::ToggleButton::textColourId,juce::Colour(0xffe9e2cf)); addAndMakeVisible(t); toggleAttachments[static_cast<size_t>(i)]=std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters,toggleIds[i],t); }
    randomButton.onClick=[this]{processor.randomizeParameters();};
    randomButton.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff8c241b)); addAndMakeVisible(randomButton);
    display.setText("32 BANKS  /  16 TABLES EACH  /  512 TOTAL",juce::dontSendNotification);
    display.setJustificationType(juce::Justification::centred);
    display.setColour(juce::Label::textColourId,juce::Colour(0xffffd34e));
    display.setFont(juce::Font(juce::FontOptions(11.0f,juce::Font::bold))); addAndMakeVisible(display);
}
void GeigerCounterProAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111212));
    auto r=getLocalBounds().toFloat().reduced(10);
    g.setColour(juce::Colour(0xff282b29)); g.fillRoundedRectangle(r,12);
    g.setColour(juce::Colour(0xffb68c30)); g.drawRoundedRectangle(r,12,1.5f);
    g.setColour(juce::Colour(0xff4b4030)); g.drawLine(25,82,735,82,1.0f);
    g.setColour(juce::Colour(0xff5b2520)); g.drawLine(25,370,735,370,1.0f);
    g.setColour(juce::Colour(0xffc53a2c)); g.setFont(juce::Font(juce::FontOptions(13.0f,juce::Font::bold)));
    g.drawText("WAVETABLE ENGINE",28,355,210,18,juce::Justification::left);
}
void GeigerCounterProAudioProcessorEditor::resized()
{
    title.setBounds(25,17,470,38); subtitle.setBounds(28,54,500,20);
    const int x0=24, y0=91, cellW=140, cellH=128;
    for(int i=0;i<10;++i){int row=i/5,col=i%5; int x=x0+col*cellW,y=y0+row*cellH;
        names[static_cast<size_t>(i)].setBounds(x,y,cellW-5,18);
        knobs[static_cast<size_t>(i)].setBounds(x+9,y+17,cellW-22,106);
    }
    display.setBounds(25,350,470,22);
    const int toggleX=28,toggleY=398;
    for(int i=0;i<7;++i){int col=i%4,row=i/4;toggles[static_cast<size_t>(i)].setBounds(toggleX+col*175,toggleY+row*42,165,28);}
    randomButton.setBounds(590,485,125,36);
}
