#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour colGraphiteDark { 0xff23272c };
    const juce::Colour colGraphiteMid  { 0xff32373e };
    const juce::Colour colGraphiteHi   { 0xff464d56 };
    const juce::Colour colTextLight    { 0xfff2f4f7 };
    const juce::Colour colTextDim      { 0xffb8c0ca };
    const juce::Colour colBtnOff       { 0xffeceff2 };
    const juce::Colour colBtnOnYellow  { 0xffffd23f };

    // Colores por etapa de la cadena
    const juce::Colour colSumAccent    { 0xff56b4e9 }; // Azul acero: Suma / I/O
    const juce::Colour colDriveAccent  { 0xff4cd97b }; // Verde: Bus Driver
    const juce::Colour colThickAccent  { 0xffc084fc }; // Violeta: Thickness
    const juce::Colour colCompAccent   { 0xffff6b57 }; // Rojo/Coral SSL: Compresor G
    const juce::Colour colSpaceAccent  { 0xff4da6ff }; // Azul: Estéreo / Salida
}

//==============================================================================
NeedleMeter::NeedleMeter (const juce::String& titleText,
                         float minVal,
                         float maxVal,
                         std::initializer_list<float> tickValues,
                         float dangerStart)
    : title (titleText),
      minV (minVal),
      maxV (maxVal),
      ticks (tickValues),
      dangerFrom (dangerStart)
{
}

void NeedleMeter::setValue (float newValue)
{
    const float clamped = juce::jlimit (minV, maxV, newValue);
    if (std::abs (clamped - value) > 0.005f)
    {
        value = clamped;
        repaint();
    }
}

void NeedleMeter::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float scale = juce::jmax (0.6f, b.getHeight() / 160.f);

    g.setColour (juce::Colour (0xfff2e6b8));
    g.fillRoundedRectangle (b, 6.f * scale);
    g.setColour (juce::Colour (0xff2b2f36));
    g.drawRoundedRectangle (b.reduced (1.f), 6.f * scale, 1.8f * scale);

    const auto pivot = juce::Point<float> (b.getCentreX(), b.getBottom() - 10.f * scale);
    const float radius = juce::jmin (b.getHeight() * 0.74f, b.getWidth() * 0.44f);
    const float a0 = 0.82f;

    auto angleFor = [this, a0] (float v) { return juce::jmap (v, minV, maxV, -a0, a0); };

    juce::Path base;
    base.addCentredArc (pivot.x, pivot.y, radius, radius, 0.f, -a0, angleFor (dangerFrom), true);
    g.setColour (juce::Colour (0xff26292e));
    g.strokePath (base, juce::PathStrokeType (1.8f * scale));

    juce::Path red;
    red.addCentredArc (pivot.x, pivot.y, radius, radius, 0.f, angleFor (dangerFrom), a0, true);
    g.setColour (juce::Colour (0xffc93b2b));
    g.strokePath (red, juce::PathStrokeType (3.2f * scale));

    g.setFont (juce::FontOptions (10.5f * scale));
    for (auto t : ticks)
    {
        const float a = angleFor (t);
        const juce::Point<float> dir (std::sin (a), -std::cos (a));
        const auto p1 = pivot + dir * radius;
        const auto p2 = pivot + dir * (radius - 7.f * scale);
        const auto lp = pivot + dir * (radius - 19.f * scale);

        g.setColour (t >= dangerFrom ? juce::Colour (0xffc93b2b) : juce::Colour (0xff22252a));
        g.drawLine (juce::Line<float> (p1, p2), 1.4f * scale);

        const juce::String txt = (t > 0.f && minV < 0.f ? "+" : "") + juce::String ((int) t);
        g.drawText (txt, juce::Rectangle<float> (28.f * scale, 13.f * scale).withCentre (lp),
                    juce::Justification::centred, false);
    }

    g.setColour (juce::Colour (0xff22252a));
    g.setFont (juce::FontOptions (10.f * scale, juce::Font::bold));
    g.drawText (title, b.reduced (8.f * scale, 4.f * scale).removeFromTop (14.f * scale),
                juce::Justification::centredLeft, false);

    const float a = angleFor (value);
    const auto tip = pivot + juce::Point<float> (std::sin (a), -std::cos (a)) * (radius - 3.f * scale);
    g.setColour (juce::Colour (0xff141619));
    g.drawLine (juce::Line<float> (pivot, tip), 2.f * scale);
    g.fillEllipse (juce::Rectangle<float> (10.f * scale, 10.f * scale).withCentre (pivot));
}

//==============================================================================
SslFusionLookAndFeel::SslFusionLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, colTextLight);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff181b1f));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff4a525d));
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff1c2025));
    setColour (juce::ComboBox::textColourId, colTextLight);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff56606e));
    setColour (juce::ComboBox::arrowColourId, colTextLight);
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff1e2227));
    setColour (juce::PopupMenu::textColourId, colTextLight);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, colBtnOnYellow);
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colour (0xff141619));
}

void SslFusionLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPosProportional, float rotaryStartAngle,
                                             float rotaryEndAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.f);
    const float diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
    const float radius = diameter * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    const juce::Colour accent = slider.findColour (juce::Slider::thumbColourId);

    // Arco de pista y progreso
    const float trackR = radius + 3.f;
    juce::Path bgArc;
    bgArc.addCentredArc (centre.x, centre.y, trackR, trackR, 0.f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff171a1e));
    g.strokePath (bgArc, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path valArc;
    valArc.addCentredArc (centre.x, centre.y, trackR, trackR, 0.f, rotaryStartAngle, angle, true);
    g.setColour (accent.withAlpha (0.85f));
    g.strokePath (valArc, juce::PathStrokeType (3.f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Faldón SSL oscuro con bisel
    juce::ColourGradient skirtGrad (juce::Colour (0xff3c424b), centre.x, centre.y - radius,
                                    juce::Colour (0xff15181c), centre.x, centre.y + radius, false);
    g.setGradientFill (skirtGrad);
    g.fillEllipse (centre.x - radius, centre.y - radius, diameter, diameter);

    g.setColour (accent.withAlpha (0.55f));
    g.drawEllipse (centre.x - radius + 0.75f, centre.y - radius + 0.75f, diameter - 1.5f, diameter - 1.5f, 1.4f);

    // Capuchón superior con color de sección
    const float capR = radius * 0.74f;
    juce::ColourGradient capGrad (accent.brighter (0.25f), centre.x, centre.y - capR,
                                  accent.darker (0.45f), centre.x, centre.y + capR, false);
    g.setGradientFill (capGrad);
    g.fillEllipse (centre.x - capR, centre.y - capR, capR * 2.f, capR * 2.f);

    // Línea indicadora blanca brillante
    juce::Path pointer;
    const float innerR = capR * 0.25f;
    const float outerR = radius * 0.94f;
    pointer.startNewSubPath (centre.x + std::sin (angle) * innerR, centre.y - std::cos (angle) * innerR);
    pointer.lineTo (centre.x + std::sin (angle) * outerR, centre.y - std::cos (angle) * outerR);
    g.setColour (colTextLight);
    g.strokePath (pointer, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SslFusionLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown)
{
    const bool isOn = button.getToggleState();
    auto area = button.getLocalBounds().toFloat().reduced (2.f);

    // Indicador luminoso cuadrado a la izquierda (blanco apagado -> amarillo brillante encendido)
    const float ledSize = juce::jmin (area.getHeight() - 4.f, 20.f);
    auto ledRect = juce::Rectangle<float> (area.getX() + 2.f, area.getCentreY() - ledSize * 0.5f, ledSize, ledSize);

    if (isOn)
    {
        g.setColour (colBtnOnYellow.withAlpha (0.35f));
        g.fillRoundedRectangle (ledRect.expanded (3.f), 5.f);

        juce::ColourGradient onGrad (juce::Colour (0xfffff099), ledRect.getCentreX(), ledRect.getY(),
                                    juce::Colour (0xffe8a800), ledRect.getCentreX(), ledRect.getBottom(), false);
        g.setGradientFill (onGrad);
        g.fillRoundedRectangle (ledRect, 3.5f);
        g.setColour (juce::Colour (0xffffe57f));
        g.drawRoundedRectangle (ledRect, 3.5f, 1.2f);
    }
    else
    {
        juce::ColourGradient offGrad (colBtnOff, ledRect.getCentreX(), ledRect.getY(),
                                     juce::Colour (0xffc7cdd4), ledRect.getCentreX(), ledRect.getBottom(), false);
        g.setGradientFill (offGrad);
        g.fillRoundedRectangle (ledRect, 3.5f);
        g.setColour (shouldDrawButtonAsHighlighted || shouldDrawButtonAsDown ? colBtnOnYellow : juce::Colour (0xff6b737e));
        g.drawRoundedRectangle (ledRect, 3.5f, 1.2f);
    }

    auto textArea = area.withTrimmedLeft (ledSize + 8.f);
    g.setColour (isOn ? colBtnOnYellow : colTextLight);
    g.setFont (juce::FontOptions (11.5f, isOn ? juce::Font::bold : juce::Font::plain));
    g.drawFittedText (button.getButtonText(), textArea.toNearestInt(), juce::Justification::centredLeft, 2);
}

void SslFusionLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.f, 0.f, (float) width, (float) height).reduced (1.f);
    const juce::Colour accent = box.findColour (juce::ComboBox::outlineColourId);

    g.setColour (juce::Colour (0xff181b20));
    g.fillRoundedRectangle (r, 4.f);
    g.setColour (accent.withAlpha (0.75f));
    g.drawRoundedRectangle (r, 4.f, 1.2f);

    const float arrowX = r.getRight() - 14.f;
    const float arrowY = r.getCentreY();
    juce::Path tri;
    tri.addTriangle (arrowX - 4.f, arrowY - 2.f, arrowX + 4.f, arrowY - 2.f, arrowX, arrowY + 3.f);
    g.setColour (colTextLight);
    g.fillPath (tri);
}

//==============================================================================
MasterEditor::MasterEditor (MasterBusProcessor& p)
    : juce::AudioProcessorEditor (p),
      proc (p),
      grMeter ("GR (dB)", 0.f, 20.f, { 0.f, 4.f, 8.f, 12.f, 16.f, 20.f }, 12.f),
      vuMeter ("OUT VU (-18 dBFS)", -20.f, 3.f, { -20.f, -10.f, -7.f, -3.f, 0.f, 3.f }, 0.f)
{
    setLookAndFeel (&lnf);

    modules[0] = { "1. SUMA / I/O",   colSumAccent,   {} };
    modules[1] = { "2. BUS DRIVER",  colDriveAccent, {} };
    modules[2] = { "3. THICKNESS",   colThickAccent, {} };
    modules[3] = { "4. SSL G COMP",  colCompAccent,  {} };
    modules[4] = { "5. ESTÉREO / VU", colSpaceAccent, {} };

    // 1. Suma pasiva + Entrada
    addKnob   (0, "inTrim",    "In Trim",    colSumAccent);
    addButton (0, "inPad",     "Pad -30dB");
    addKnob   (0, "recovery",  "Recovery",   colSumAccent);
    addKnob   (0, "tolerance", "Tolerance",  colSumAccent);
    addKnob   (0, "crosstalk", "Crosstalk",  colSumAccent);

    // 2. Bus Driver + Salida
    addCombo  (1, "driveMode",    "Modo",       colDriveAccent);
    addKnob   (1, "drive",        "Drive",      colDriveAccent);
    addButton (1, "boost",        "Boost +19");
    addKnob   (1, "ironStrength", "Iron Str",   colDriveAccent);
    addKnob   (1, "ironMorph",    "Iron Morph", colDriveAccent);
    addButton (1, "levelMatch",   "Lvl Match");
    addButton (1, "outPad",       "Pad -18dB");
    addKnob   (1, "outTrim",      "Out Trim",   colDriveAccent);

    // 3. Thickness
    addButton (2, "enhance",   "Thickness");
    addKnob   (2, "enhAmount", "Amount",    colThickAccent);
    addKnob   (2, "wetDry",    "Wet / Dry", colThickAccent);

    // 4. Compresor SSL G
    addButton (3, "compOn",    "Comp On");
    addKnob   (3, "threshold", "Thresh",    colCompAccent);
    addCombo  (3, "ratio",     "Ratio",     colCompAccent);
    addCombo  (3, "attack",    "Attack",    colCompAccent);
    addCombo  (3, "release",   "Release",   colCompAccent);
    addCombo  (3, "scHpf",     "SC HPF",    colCompAccent);
    addKnob   (3, "vcaColor",  "VCA Col",   colCompAccent);
    addKnob   (3, "makeup",    "Makeup",    colCompAccent);

    // 5. Estéreo + 2 VU meters
    addKnob   (4, "space",    "Space",      colSpaceAccent);
    addKnob   (4, "monoFreq", "Mono Maker", colSpaceAccent);

    addAndMakeVisible (grMeter);
    addAndMakeVisible (vuMeter);

    for (auto* lbl : { &grReadout, &vuReadout })
    {
        lbl->setJustificationType (juce::Justification::centred);
        lbl->setColour (juce::Label::textColourId, colTextLight);
        addAndMakeVisible (*lbl);
    }

    // Escalado proporcional arrastrando desde la esquina inferior derecha
    constrainer.setFixedAspectRatio ((double) baseWidth / (double) baseHeight);
    constrainer.setSizeLimits (820, 332, 2760, 1120);
    setResizable (true, true);
    addAndMakeVisible (resizer);

    setSize (baseWidth, baseHeight);
    startTimerHz (30);
}

MasterEditor::~MasterEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void MasterEditor::addKnob (int modIdx, const juce::String& id, const juce::String& text, juce::Colour accent)
{
    auto slot = std::make_unique<KnobSlot>();
    slot->paramId = id;
    slot->labelText = text;
    slot->accent = accent;
    slot->moduleIndex = modIdx;

    slot->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slot->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 17);
    slot->slider.setColour (juce::Slider::thumbColourId, accent);
    slot->slider.setColour (juce::Slider::textBoxOutlineColourId, accent.withAlpha (0.65f));
    addAndMakeVisible (slot->slider);

    slot->label.setText (text, juce::dontSendNotification);
    slot->label.setJustificationType (juce::Justification::centred);
    slot->label.setColour (juce::Label::textColourId, colTextDim);
    addAndMakeVisible (slot->label);

    slot->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, id, slot->slider);

    knobs.push_back (std::move (slot));
}

void MasterEditor::addButton (int modIdx, const juce::String& id, const juce::String& text)
{
    auto slot = std::make_unique<ButtonSlot>();
    slot->paramId = id;
    slot->labelText = text;
    slot->moduleIndex = modIdx;

    slot->button.setButtonText (text);
    slot->button.setClickingTogglesState (true);
    addAndMakeVisible (slot->button);

    slot->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        proc.apvts, id, slot->button);

    buttons.push_back (std::move (slot));
}

void MasterEditor::addCombo (int modIdx, const juce::String& id, const juce::String& text, juce::Colour accent)
{
    auto slot = std::make_unique<ComboSlot>();
    slot->paramId = id;
    slot->labelText = text;
    slot->accent = accent;
    slot->moduleIndex = modIdx;

    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (proc.apvts.getParameter (id)))
        slot->box.addItemList (choice->choices, 1);

    slot->box.setColour (juce::ComboBox::outlineColourId, accent);
    addAndMakeVisible (slot->box);

    slot->label.setText (text, juce::dontSendNotification);
    slot->label.setJustificationType (juce::Justification::centred);
    slot->label.setColour (juce::Label::textColourId, colTextDim);
    addAndMakeVisible (slot->label);

    slot->attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.apvts, id, slot->box);

    combos.push_back (std::move (slot));
}

void MasterEditor::drawBrushedGraphite (juce::Graphics& g, juce::Rectangle<float> area)
{
    juce::ColourGradient chassisGrad (colGraphiteMid, area.getCentreX(), area.getY(),
                                      colGraphiteDark, area.getCentreX(), area.getBottom(), false);
    g.setGradientFill (chassisGrad);
    g.fillRoundedRectangle (area, 8.f);

    // Vetas horizontales de metal cepillado
    g.saveState();
    g.reduceClipRegion (area.toNearestInt());
    juce::Random rng (4201);
    for (int i = 0; i < 240; ++i)
    {
        const float y = area.getY() + rng.nextFloat() * area.getHeight();
        const float x0 = area.getX() + rng.nextFloat() * area.getWidth();
        const float len = 35.f + rng.nextFloat() * 140.f;
        const bool bright = rng.nextBool();
        g.setColour ((bright ? colGraphiteHi : juce::Colour (0xff121518)).withAlpha (0.09f));
        g.drawHorizontalLine ((int) y, x0, juce::jmin (area.getRight(), x0 + len));
    }
    g.restoreState();

    g.setColour (juce::Colour (0xff5c6572));
    g.drawRoundedRectangle (area.reduced (0.5f), 8.f, 1.2f);
}

void MasterEditor::drawRackEars (juce::Graphics& g, juce::Rectangle<float> area)
{
    const float earW = 28.f;
    const juce::Rectangle<float> ears[2] {
        { area.getX(), area.getY(), earW, area.getHeight() },
        { area.getRight() - earW, area.getY(), earW, area.getHeight() }
    };

    for (const auto& ear : ears)
    {
        g.setColour (juce::Colour (0xff1a1d22));
        g.fillRoundedRectangle (ear, 6.f);

        const float screwX = ear.getCentreX();
        const float ys[2] { ear.getY() + 34.f, ear.getBottom() - 34.f };
        for (float sy : ys)
        {
            g.setColour (juce::Colour (0xff67717e));
            g.fillEllipse (screwX - 6.f, sy - 6.f, 12.f, 12.f);
            g.setColour (juce::Colour (0xff181b20));
            g.drawLine (screwX - 4.f, sy - 2.f, screwX + 4.f, sy + 2.f, 1.5f);
        }
    }
}

void MasterEditor::paint (juce::Graphics& g)
{
    const auto full = getLocalBounds().toFloat();
    drawBrushedGraphite (g, full);
    drawRackEars (g, full);

    const float scale = (float) getWidth() / (float) baseWidth;

    for (const auto& mod : modules)
    {
        if (mod.bounds.isEmpty())
            continue;

        const auto mb = mod.bounds.toFloat();
        g.setColour (juce::Colour (0xff171a1e).withAlpha (0.42f));
        g.fillRoundedRectangle (mb, 6.f);

        g.setColour (mod.accent.withAlpha (0.45f));
        g.drawRoundedRectangle (mb.reduced (0.5f), 6.f, 1.f);

        auto headerArea = mb;
        auto header = headerArea.removeFromTop (24.f * scale);
        g.setColour (mod.accent.withAlpha (0.18f));
        g.fillRoundedRectangle (header, 5.f);

        g.setColour (mod.accent.brighter (0.25f));
        g.setFont (juce::FontOptions (11.5f * scale, juce::Font::bold));
        g.drawText (mod.title, header.reduced (8.f * scale, 0.f), juce::Justification::centredLeft, false);
    }
}

void MasterEditor::paintOverChildren (juce::Graphics& g)
{
    const float scale = (float) getWidth() / (float) baseWidth;
    g.setColour (colTextDim);
    g.setFont (juce::FontOptions (10.f * scale, juce::Font::bold));
    g.drawText ("DG-MASTERBUS  //  ARRASTRA ESQUINA PARA ESCALAR",
                getLocalBounds().reduced (36, 4).removeFromBottom (16),
                juce::Justification::centredRight, false);
}

void MasterEditor::resized()
{
    const float s = (float) getWidth() / (float) baseWidth;
    auto S = [s] (float v) { return juce::roundToInt (v * s); };

    auto rack = getLocalBounds().reduced (S (34), S (12));
    const int gap = S (8);
    const int totalW = rack.getWidth() - gap * 4;

    // Anchos proporcionales al flujo de señal:
    // Suma(180) | BusDriver(340) | Thickness(170) | CompG(350) | Estéreo+VU(240) -> base 1280 + gaps
    const int w0 = juce::roundToInt (totalW * (180.f / 1280.f));
    const int w1 = juce::roundToInt (totalW * (340.f / 1280.f));
    const int w2 = juce::roundToInt (totalW * (170.f / 1280.f));
    const int w3 = juce::roundToInt (totalW * (350.f / 1280.f));
    const int w4 = juce::jmax (120, rack.getWidth() - (w0 + w1 + w2 + w3 + gap * 4));

    const int widths[5] { w0, w1, w2, w3, w4 };
    int cx = rack.getX();

    for (int i = 0; i < 5; ++i)
    {
        modules[i].bounds = { cx, rack.getY(), widths[i], rack.getHeight() };
        cx += widths[i] + gap;
    }

    // Colocar controles dentro de cada módulo
    auto placeInModule = [&] (int modIdx)
    {
        auto inner = modules[modIdx].bounds.reduced (S (8), S (8));
        inner.removeFromTop (S (22)); // cabecera del módulo

        std::vector<KnobSlot*> mKnobs;
        std::vector<ButtonSlot*> mBtns;
        std::vector<ComboSlot*> mCombos;

        for (auto& k : knobs)   if (k->moduleIndex == modIdx) mKnobs.push_back (k.get());
        for (auto& b : buttons) if (b->moduleIndex == modIdx) mBtns.push_back (b.get());
        for (auto& c : combos)  if (c->moduleIndex == modIdx) mCombos.push_back (c.get());

        // Zona superior para combos y botones
        const int topRowH = S (44);
        int topItems = (int) mCombos.size() + (int) mBtns.size();
        if (topItems > 0 && inner.getHeight() > topRowH + S (80))
        {
            auto topArea = inner.removeFromTop (topRowH);
            const int cols = juce::jmin (topItems, 4);
            const int cellW = topArea.getWidth() / juce::jmax (1, cols);

            int idx = 0;
            for (auto* c : mCombos)
            {
                const int col = idx % cols;
                const int row = idx / cols;
                auto cell = juce::Rectangle<int> (topArea.getX() + col * cellW,
                                                  topArea.getY() + row * S (44),
                                                  cellW, S (42)).reduced (S (2));
                c->label.setBounds (cell.removeFromTop (S (15)));
                c->label.setFont (juce::FontOptions (10.5f * s));
                c->box.setBounds (cell);
                ++idx;
            }

            for (auto* b : mBtns)
            {
                const int col = idx % cols;
                const int row = idx / cols;
                auto cell = juce::Rectangle<int> (topArea.getX() + col * cellW,
                                                  topArea.getY() + row * S (44),
                                                  cellW, S (42)).reduced (S (2));
                b->button.setBounds (cell);
                ++idx;
            }

            if (topItems > cols)
                inner.removeFromTop (S (44));
        }

        // Zona restante para perillas en cuadrícula
        if (! mKnobs.empty())
        {
            const int nK = (int) mKnobs.size();
            const int cols = (nK <= 2) ? 1 : ((inner.getWidth() >= S (240)) ? 4 : 2);
            const int rows = (nK + cols - 1) / cols;
            const int cellW = inner.getWidth() / cols;
            const int cellH = inner.getHeight() / juce::jmax (1, rows);

            for (int i = 0; i < nK; ++i)
            {
                const int col = i % cols;
                const int row = i / cols;
                auto cell = juce::Rectangle<int> (inner.getX() + col * cellW,
                                                  inner.getY() + row * cellH,
                                                  cellW, cellH).reduced (S (3));

                auto* k = mKnobs[i];
                k->label.setBounds (cell.removeFromTop (S (16)));
                k->label.setFont (juce::FontOptions (10.5f * s));

                auto tbBounds = cell.removeFromBottom (S (18));
                k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false,
                                           juce::jmax (44, cell.getWidth() - S (6)), S (16));
                k->slider.setBounds (cell.getX(), cell.getY(), cell.getWidth(), cell.getHeight() + S (16));
                juce::ignoreUnused (tbBounds);
            }
        }
    };

    for (int i = 0; i < 4; ++i)
        placeInModule (i);

    // Módulo 5: Space + Mono Maker arriba y los 2 VU meters debajo
    {
        auto mod5 = modules[4].bounds.reduced (S (8), S (8));
        mod5.removeFromTop (S (22));

        auto topKnobs = mod5.removeFromTop (S (112));
        const int halfW = topKnobs.getWidth() / 2;

        auto setupStereoKnob = [&] (const juce::String& id, juce::Rectangle<int> area)
        {
            for (auto& k : knobs)
            {
                if (k->paramId == id)
                {
                    k->label.setBounds (area.removeFromTop (S (16)));
                    k->label.setFont (juce::FontOptions (10.5f * s));
                    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false,
                                               juce::jmax (44, area.getWidth() - S (4)), S (16));
                    k->slider.setBounds (area.getX(), area.getY(), area.getWidth(), area.getHeight() + S (16));
                }
            }
        };

        setupStereoKnob ("space",    topKnobs.withWidth (halfW).reduced (S (2)));
        setupStereoKnob ("monoFreq", topKnobs.withX (topKnobs.getX() + halfW).withWidth (halfW).reduced (S (2)));

        mod5.removeFromTop (S (6));
        const int meterBlockH = (mod5.getHeight() - S (6)) / 2;
        const int readoutH = S (20);

        auto grArea = mod5.removeFromTop (meterBlockH);
        grReadout.setBounds (grArea.removeFromBottom (readoutH));
        grReadout.setFont (juce::FontOptions (10.5f * s, juce::Font::bold));
        grMeter.setBounds (grArea.reduced (S (2)));

        mod5.removeFromTop (S (6));
        auto vuArea = mod5;
        vuReadout.setBounds (vuArea.removeFromBottom (readoutH));
        vuReadout.setFont (juce::FontOptions (10.5f * s, juce::Font::bold));
        vuMeter.setBounds (vuArea.reduced (S (2)));
    }

    const int rzSize = S (18);
    resizer.setBounds (getWidth() - rzSize, getHeight() - rzSize, rzSize, rzSize);
}

void MasterEditor::timerCallback()
{
    const float gr = proc.grMeterDb.load();
    grShown = gr > grShown ? gr : grShown + (gr - grShown) * 0.25f;
    grMeter.setValue (grShown);

    if (gr >= grPeak)
    {
        grPeak = gr;
        grHold = 60;
    }
    else if (--grHold <= 0)
    {
        grPeak = juce::jmax (0.f, grPeak - 0.3f);
        grHold = 0;
    }

    grReadout.setText ("GR " + juce::String (grShown, 1) + " dB  |  Pico " + juce::String (grPeak, 1) + " dB",
                       juce::dontSendNotification);

    const float db = juce::Decibels::gainToDecibels (proc.vuLevel.load(), -100.f);
    const float vu = db + 18.f;
    vuMeter.setValue (vu);
    vuReadout.setText (juce::String (db, 1) + " dBFS  (" + juce::String (vu, 1) + " VU)",
                       juce::dontSendNotification);
}
