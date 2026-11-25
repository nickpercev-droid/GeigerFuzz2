#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour black(0xff080808), panel(0xff151313), red(0xffd51e16),
                       hotRed(0xffff3826), bone(0xffd1c8bb), amber(0xffffb34d);

    void scratch(juce::Graphics& g, int seed, int width, int height)
    {
        juce::Random r(seed);
        for (int i = 0; i < 430; ++i)
        {
            float x = r.nextFloat() * width, y = r.nextFloat() * height;
            float len = 2.0f + r.nextFloat() * 26.0f;
            g.setColour((i % 4 == 0 ? red : bone).withAlpha(0.035f + r.nextFloat()*0.09f));
            g.drawLine(x, y, x + len, y + (r.nextFloat()-0.5f)*2.0f, 0.5f + r.nextFloat());
        }
        for (int i = 0; i < 650; ++i)
        {
            float x = r.nextFloat()*width, y = r.nextFloat()*height;
            g.setColour((i%5==0 ? red : bone).withAlpha(0.025f+r.nextFloat()*0.07f));
            g.fillEllipse(x,y,1.0f+r.nextFloat()*2.0f,1.0f+r.nextFloat()*2.0f);
        }
    }
}

HellfurnaceLookAndFeel::HellfurnaceLookAndFeel()
{
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff090909));
    setColour(juce::ComboBox::textColourId, bone);
    setColour(juce::ComboBox::outlineColourId, red);
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, bone);
}

void HellfurnaceLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                               float pos, float start, float end, juce::Slider&)
{
    auto area = juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(7.0f);
    float diameter = juce::jmin(area.getWidth(), area.getHeight()) * 0.78f;
    auto r = juce::Rectangle<float>(0,0,diameter,diameter).withCentre(area.getCentre());
    auto c = r.getCentre(); float radius = diameter*0.5f;
    g.setColour(juce::Colours::black.withAlpha(0.9f));
    g.fillEllipse(r.expanded(5));
    g.setColour(juce::Colour(0xff292727));
    g.fillEllipse(r.expanded(1));
    g.setColour(juce::Colour(0xff77716b));
    g.drawEllipse(r.expanded(1), 1.2f);
    for (int i=0;i<=20;++i)
    {
        float a = start + (end-start)*(i/20.0f);
        auto p1 = c + juce::Point<float>(std::sin(a)*(radius+5),-std::cos(a)*(radius+5));
        auto p2 = c + juce::Point<float>(std::sin(a)*(radius+12),-std::cos(a)*(radius+12));
        g.setColour(i/20.0f <= pos ? red : juce::Colour(0xff77716b));
        g.drawLine({p1,p2}, i%5==0 ? 2.2f : 1.1f);
    }
    juce::ColourGradient grad(juce::Colour(0xff55504c), r.getX(),r.getY(),
                              juce::Colour(0xff171616),r.getRight(),r.getBottom(),false);
    g.setGradientFill(grad); g.fillEllipse(r.reduced(5));
    g.setColour(juce::Colour(0xff77716b)); g.drawEllipse(r.reduced(5),1.0f);
    float a = start + pos*(end-start);
    juce::Point<float> tip(c.x+std::sin(a)*radius*0.72f,c.y-std::cos(a)*radius*0.72f);
    g.setColour(hotRed); g.drawLine(c.x,c.y,tip.x,tip.y,6.0f);
    g.setColour(juce::Colour(0xffff9a76)); g.fillEllipse(tip.x-2.3f,tip.y-2.3f,4.6f,4.6f);
    g.setColour(juce::Colours::black.withAlpha(0.7f)); g.fillEllipse(c.x-5,c.y-5,10,10);
}

HellfurnaceFuzzAudioProcessorEditor::HellfurnaceFuzzAudioProcessorEditor(HellfurnaceFuzzAudioProcessor& p)
: AudioProcessorEditor(&p), processor(p)
{
    setLookAndFeel(&lookAndFeel);
    setSize(900, 590);
    for (size_t i=0;i<knobs.size();++i)
    {
        auto& s=knobs[i];
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,62,18);
        s.setColour(juce::Slider::textBoxTextColourId,bone);
        s.setColour(juce::Slider::textBoxBackgroundColourId,black);
        addAndMakeVisible(s);
        labels[i].setText(ids[i],juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId,bone);
        labels[i].setFont(juce::Font(juce::FontOptions(17.0f,juce::Font::bold)));
        addAndMakeVisible(labels[i]);
        attachments[i]=std::make_unique<SliderAttachment>(processor.parameters,ids[i],s);
    }
    modeBox.addItem("Classic",1); modeBox.addItem("Searing",2); modeBox.addItem("Chaos Max",3);
    addAndMakeVisible(modeBox);
    modeAttachment=std::make_unique<APVTS::ComboBoxAttachment>(processor.parameters,"MODE",modeBox);
    powerButton.setClickingTogglesState(true);
    powerButton.setColour(juce::ToggleButton::textColourId,hotRed);
    powerButton.setColour(juce::ToggleButton::tickColourId,hotRed);
    addAndMakeVisible(powerButton);
    powerAttachment=std::make_unique<APVTS::ButtonAttachment>(processor.parameters,"ON",powerButton);
    startTimerHz(30);
}

HellfurnaceFuzzAudioProcessorEditor::~HellfurnaceFuzzAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void HellfurnaceFuzzAudioProcessorEditor::timerCallback()
{
    meter=juce::jmax(processor.getMeterLevel(),meter*0.82f);
    repaint(650,500,230,75);
}

void HellfurnaceFuzzAudioProcessorEditor::drawSkull(juce::Graphics& g,juce::Rectangle<float> b)
{
    auto cx=b.getCentreX(), cy=b.getCentreY();
    g.setColour(juce::Colour(0xffb9b1a7).withAlpha(0.8f));
    juce::Path skull;
    skull.addEllipse(cx-b.getWidth()*0.36f,cy-b.getHeight()*0.40f,b.getWidth()*0.72f,b.getHeight()*0.72f);
    g.fillPath(skull);
    g.setColour(black); g.fillEllipse(cx-b.getWidth()*0.27f,cy-b.getHeight()*0.05f,b.getWidth()*0.20f,b.getHeight()*0.20f);
    g.fillEllipse(cx+b.getWidth()*0.07f,cy-b.getHeight()*0.05f,b.getWidth()*0.20f,b.getHeight()*0.20f);
    juce::Path nose; nose.startNewSubPath(cx,cy+b.getHeight()*0.08f);
    nose.lineTo(cx-b.getWidth()*0.07f,cy+b.getHeight()*0.23f);
    nose.lineTo(cx+b.getWidth()*0.07f,cy+b.getHeight()*0.23f); nose.closeSubPath(); g.fillPath(nose);
    g.setColour(juce::Colour(0xffd0c7bb)); g.fillRoundedRectangle(cx-b.getWidth()*0.22f,cy+b.getHeight()*0.28f,b.getWidth()*0.44f,b.getHeight()*0.24f,5);
    g.setColour(black);
    for(int i=0;i<7;++i) g.drawLine(cx-b.getWidth()*0.19f+i*b.getWidth()*0.063f,cy+b.getHeight()*0.29f,
                                    cx-b.getWidth()*0.19f+i*b.getWidth()*0.063f,cy+b.getHeight()*0.50f,2.0f);
}

void HellfurnaceFuzzAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(black);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff282321),0,0,black,0,(float)getHeight(),false));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(3),12);
    scratch(g,77,getWidth(),getHeight());
    g.setColour(red); g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(4),12,3);
    g.setColour(bone); g.setFont(juce::Font(juce::FontOptions(49.0f,juce::Font::bold)));
    g.drawText("HELLFURNACE",28,8,540,58,juce::Justification::centredLeft);
    g.setColour(hotRed); g.drawText("FUZZ",555,8,210,58,juce::Justification::centredLeft);
    g.setColour(bone); g.setFont(juce::Font(juce::FontOptions(16.0f)));
    g.drawText("ULTRA DIRTY GUITAR FUZZ",250,62,420,24,juce::Justification::centred);
    g.setColour(red); g.drawLine(12,91,getWidth()-12,91,2.0f);
    g.setColour(bone); g.setFont(juce::Font(juce::FontOptions(13.0f,juce::Font::bold)));
    g.drawText("MODE",690,18,150,22,juce::Justification::centred);
    g.setColour(juce::Colour(0xff4a1712)); g.drawRoundedRectangle(680,38,190,40,7,2);
    g.setColour(red); g.drawLine(12,476,getWidth()-12,476,2.0f);
    drawSkull(g,{755,170,120,260});
    g.setColour(hotRed); g.setFont(juce::Font(juce::FontOptions(24.0f,juce::Font::bold|juce::Font::italic)));
    g.drawText("MORE FUZZ!  MORE CHAOS!",205,505,440,45,juce::Justification::centred);
    g.setColour(juce::Colour(0xff35100d)); g.drawRoundedRectangle(650,490,230,80,8,2);
    g.setColour(juce::Colour(0xff1a0807)); g.fillRoundedRectangle(657,497,216,66,5);
    for(int i=0;i<18;++i)
    {
        float level=juce::jlimit(0.04f,1.0f,meter*(0.65f+0.35f*std::sin(i*0.7f+1.0f)));
        float bh=8.0f+level*48.0f;
        g.setColour(i<12?red:hotRed);
        g.fillRect(665.0f+i*11.0f,555.0f-bh,7.0f,bh);
    }
}

void HellfurnaceFuzzAudioProcessorEditor::resized()
{
    const int left=25, top=112, gap=12, w=145, h=164;
    for(int i=0;i<8;++i)
    {
        int row=i/4,col=i%4;
        int x=left+col*(w+gap), y=top+row*177;
        labels[(size_t)i].setBounds(x,y,w,25);
        knobs[(size_t)i].setBounds(x+5,y+25,w-10,h-25);
    }
    modeBox.setBounds(690,42,170,32);
    powerButton.setBounds(38,510,125,45);
}
