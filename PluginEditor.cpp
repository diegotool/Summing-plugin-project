#include "PluginEditor.h"

//==============================================================================
void NeedleMeter::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();

    // Esfera
    g.setColour (juce::Colour (0xfff2e6b8));
    g.fillRoundedRectangle (b, 8.f);
    g.setColour (juce::Colour (0xff3a3a3a));
    g.drawRoundedRectangle (b.reduced (1.f), 8.f, 2.f);

    const auto pivot = juce::Point<float> (b.getCentreX(), b.getBottom() - 14.f);
    const float radius = juce::jmin (b.getHeight() * 0.80f, b.getWidth() * 0.46f);
    const float a0 = 0.85f; // rad a cada lado de la vertical

    auto angleFor = [this, a0] (float v) { return juce::jmap (v, minV, maxV, -a0, a0); };

    // Arco normal y zona roja
    {
        juce::Path base;
        base.addCentredArc (pivot.x, pivot.y, radius, radius, 0.f, -a0, angleFor (dangerFrom), true);
        g.setColour (juce::Colour (0xff2b2b2b));
        g.strokePath (base, juce::PathStrokeType (2.f));

        juce::Path red;
        red.addCentredArc (pivot.x, pivot.y, radius, radius, 0.f, angleFor (dangerFrom), a0, true);
        g.setColour (juce::Colour (0xffc0392b));
        g.strokePath (red, juce::PathStrokeType (4.f));
    }

    // Marcas y números
    g.setFont (13.f);
    for (auto t : ticks)
    {
        const float a = angleFor (t);
        const juce::Point<float> dir (std::sin (a), -std::cos (a));
        const auto p1 = pivot + dir * radius;
        const auto p2 = pivot + dir * (radius - 9.f);
        const auto lp = pivot + dir * (radius - 24.f);

        g.setColour (t >= dangerFrom ? juce::Colour (0xffc0392b) : juce::Colour (0xff2b2b2b));
        g.drawLine (juce::Line<float> (p1, p2), 1.6f);

        const juce::String text = (t > 0.f && minV < 0.f ? "+" : "") + juce::String ((int) t);
        g.drawText (text, juce::Rectangle<float> (32.f, 16.f).withCentre (lp),
                    juce::Justification::centred, false);
    }

    // Título
    g.setColour (juce::Colour (0xff2b2b2b));
    g.setFont (12.f);
    g.drawText (title, juce::Rectangle<float> (b.getX() + 10.f, b.getY() + 6.f, b.getWidth() - 20.f, 16.f),
                juce::Justification::centredLeft, false);

    // Aguja
    const float a = angleFor (value);
    const auto tip = pivot + juce::Point<float> (std::sin (a), -std::cos (a)) * (radius - 4.f);
    g.setColour (juce::Colour (0xff111111));
    g.drawLine (juce::Line<float> (pivot, tip), 2.2f);
    g.fillEllipse (juce::Rectangle<float> (12.f, 12.f).withCentre (pivot));
}

//==============================================================================
MasterEditor::MasterEditor (MasterBusProcessor& p)
    : juce::AudioProcessorEditor (p),
      proc (p),
      grMeter ("GAIN REDUCTION  (dB)", 0.f, 20.f, { 0.f, 2.f, 4.f, 6.f, 8.f, 10.f, 15.f, 20.f }, 12.f),
      vuMeter ("OUTPUT VU  (0 VU = -18 dBFS)", -20.f, 3.f, { -20.f, -10.f, -7.f, -5.f, -3.f, 0.f, 3.f }, 0.f)
{
    addAndMakeVisible (grMeter);
    addAndMakeVisible (vuMeter);

    for (auto* l : { &grReadout, &vuReadout })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::textColourId, juce::Colours::white);
        l->setFont (juce::FontOptions (15.f));
        addAndMakeVisible (*l);
    }

    addSection ("ENTRADA / SALIDA");
    addSlider ("inTrim",  "Input Trim");
    addSlider ("outTrim", "Output Trim");

    addSection ("SUMA PASIVA (MINI-BUS)");
    addSlider ("recovery",  "Gain Recovery");
    addSlider ("tolerance", "Tolerance");
    addSlider ("crosstalk", "Crosstalk");

    addSection ("BUS DRIVER");
    addCombo  ("driveMode", "Drive Mode");
    addSlider ("drive", "Drive");
    addToggle ("boost", "Boost (+19 dB)");
    addToggle ("inPad", "Input Pad (-30 dB)");
    addToggle ("outPad", "Output Pad (-18 dB)");
    addToggle ("levelMatch", "Level Match");
    addSlider ("ironStrength", "Iron Strength");
    addSlider ("ironMorph", "Iron Morph");

    addSection ("THICKNESS");
    addToggle ("enhance", "Thickness On");
    addSlider ("enhAmount", "Thickness Amount");
    addSlider ("wetDry", "Wet / Dry");

    addSection ("ESTEREO / GRAVES");
    addSlider ("space", "Space");
    addSlider ("monoFreq", "Mono Maker");

    addSection ("COMPRESOR (SSL G)");
    addToggle ("compOn", "Comp On");
    addSlider ("threshold", "Threshold");
    addCombo  ("ratio", "Ratio");
    addCombo  ("attack", "Attack");
    addCombo  ("release", "Release");
    addSlider ("makeup", "Makeup");
    addCombo  ("scHpf", "SC HPF");
    addSlider ("vcaColor", "VCA Color");

    setSize (1110, 710);
    startTimerHz (30);
}

MasterEditor::~MasterEditor()
{
    stopTimer();
}

void MasterEditor::addSection (const juce::String& text)
{
    auto l = std::make_unique<juce::Label> ("", text);
    l->setColour (juce::Label::textColourId, juce::Colour (0xff8fd3f4));
    l->setFont (juce::FontOptions (15.f, juce::Font::bold));
    addAndMakeVisible (*l);
    layoutRows.push_back (Row { l.get(), nullptr, true });
    headers.push_back (std::move (l));
}

void MasterEditor::addSlider (const juce::String& id, const juce::String& text)
{
    auto row = std::make_unique<SliderRow>();
    row->slider.setSliderStyle (juce::Slider::LinearHorizontal);
    row->slider.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 22);
    row->label.setText (text, juce::dontSendNotification);
    row->label.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (row->label);
    addAndMakeVisible (row->slider);
    row->att = std::make_unique<SliderAtt> (proc.apvts, id, row->slider);
    layoutRows.push_back (Row { &row->label, &row->slider, false });
    sliders.push_back (std::move (row));
}

void MasterEditor::addCombo (const juce::String& id, const juce::String& text)
{
    auto row = std::make_unique<ComboRow>();
    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        row->box.addItemList (choice->choices, 1);
    row->label.setText (text, juce::dontSendNotification);
    row->label.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (row->label);
    addAndMakeVisible (row->box);
    row->att = std::make_unique<ComboAtt> (proc.apvts, id, row->box);
    layoutRows.push_back (Row { &row->label, &row->box, false });
    combos.push_back (std::move (row));
}

void MasterEditor::addToggle (const juce::String& id, const juce::String& text)
{
    auto row = std::make_unique<ToggleRow>();
    row->button.setButtonText (text);
    row->button.setColour (juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible (row->button);
    row->att = std::make_unique<ButtonAtt> (proc.apvts, id, row->button);
    layoutRows.push_back (Row { nullptr, &row->button, false });
    toggles.push_back (std::move (row));
}

void MasterEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff262b30));
}

void MasterEditor::resized()
{
    auto area = getLocalBounds().reduced (12);

    auto left = area.removeFromLeft (310);
    grMeter.setBounds (left.removeFromTop (210));
    left.removeFromTop (6);
    grReadout.setBounds (left.removeFromTop (28));
    left.removeFromTop (14);
    vuMeter.setBounds (left.removeFromTop (210));
    left.removeFromTop (6);
    vuReadout.setBounds (left.removeFromTop (28));

    area.removeFromLeft (14);

    // Controles en dos columnas
    const int colGap = 16;
    const int colW = (area.getWidth() - colGap) / 2;
    int x = area.getX();
    const int top = area.getY();
    const int bottom = area.getBottom();
    int y = top;

    for (const auto& r : layoutRows)
    {
        const int h = r.isHeader ? 26 : 28;
        int gap = (r.isHeader && y != top) ? 6 : 0;
        const int need = r.isHeader ? h + 2 * 28 : h; // una cabecera nunca queda sola al final

        if (y + gap + need > bottom)
        {
            x += colW + colGap;
            y = top;
            gap = 0;
        }

        y += gap;

        if (r.isHeader)
        {
            r.label->setBounds (x, y, colW, h);
        }
        else if (r.label != nullptr)
        {
            r.label->setBounds (x, y, 130, h);
            r.control->setBounds (x + 130, y + 2, colW - 130, h - 4);
        }
        else
        {
            r.control->setBounds (x + 4, y + 2, colW - 4, h - 4);
        }

        y += h;
    }
}

void MasterEditor::timerCallback()
{
    // Reducción de ganancia: ataque instantáneo, caída rápida; con retención del pico
    const float gr = proc.grMeterDb.load();
    grShown = gr > grShown ? gr : grShown + (gr - grShown) * 0.25f;
    grMeter.setValue (grShown);

    if (gr >= grPeak)
    {
        grPeak = gr;
        grHold = 60; // ~2 s
    }
    else if (--grHold <= 0)
    {
        grPeak = juce::jmax (0.f, grPeak - 0.3f);
        grHold = 0;
    }

    grReadout.setText ("GR " + juce::String (grShown, 1) + " dB     pico " + juce::String (grPeak, 1) + " dB",
                       juce::dontSendNotification);

    // VU de salida (0 VU = -18 dBFS)
    const float db = juce::Decibels::gainToDecibels (proc.vuLevel.load(), -100.f);
    const float vu = db + 18.f;
    vuMeter.setValue (vu);
    vuReadout.setText (juce::String (db, 1) + " dBFS   (" + juce::String (vu, 1) + " VU)",
                       juce::dontSendNotification);
}
