#include "SenderEditor.h"

//==============================================================================
SenderPanel::SenderPanel (SenderProcessor& p)
    : kBus   (p.apvts, "bus",       "BUS",       dg::pal::sand,   "", { "DRUMS", "GTRS", "KEYS", "VOX", "FX" }),
      kTol   (p.apvts, "tolerance", "TOLERANCE", dg::pal::copper),
      kColor (p.apvts, "color",     "COLOR",     dg::pal::teal,   "dB"),
      kMode  (p.apvts, "mode",      "MODE",      dg::pal::teal,   "", { "DISC", "TAPE", "GRIT" }),
      kWidth (p.apvts, "width",     "WIDTH",     dg::pal::peri,   "%"),
      kTrim  (p.apvts, "trim",      "TRIM",      dg::pal::sand,   "dB")
{
    setLookAndFeel (&laf);
    brushed = dg::makeBrushed (baseW, baseH);

    kMode.displayOverride = [] (double v)
    {
        static const char* names[] = { "CLASS A", "TAPE", "GRIT" };
        return juce::String (names[juce::jlimit (0, 2, juce::roundToInt (v))]);
    };

    kBus.displayOverride = [] (double v)
    {
        static const char* names[] = { "DRUMS", "GUITARS", "KEYS", "VOCALS", "FX" };
        return juce::String (names[juce::jlimit (0, 4, juce::roundToInt (v))]);
    };

    for (auto* k : { &kBus, &kTol, &kColor, &kMode, &kWidth, &kTrim })
        addAndMakeVisible (*k);

    setSize (baseW, baseH);
}

SenderPanel::~SenderPanel()
{
    setLookAndFeel (nullptr);
}

void SenderPanel::resized()
{
    sections.clear();

    constexpr int y = 72, rowH = 176, ky = 34, kw = 80, kh = 126;

    auto addSection = [this] (const char* title, juce::Colour accent, int x, int w)
    {
        sections.push_back ({ title, accent, { x, y, w, rowH } });
    };

    auto place = [] (dg::Knob& k, int x, int yy) { k.setBounds (x, yy, kw, kh); };

    int x = 36;

    addSection ("BUS", dg::pal::sand, x, 104);
    place (kBus, x + 12, y + ky);
    x += 112;

    addSection ("COLOUR", dg::pal::teal, x, 264);
    place (kTol,   x + 12,  y + ky);
    place (kColor, x + 92,  y + ky);
    place (kMode,  x + 172, y + ky);
    x += 272;

    addSection ("STEREO / LEVEL", dg::pal::peri, x, 184);
    place (kWidth, x + 12, y + ky);
    place (kTrim,  x + 92, y + ky);
}

void SenderPanel::paint (juce::Graphics& g)
{
    dg::drawChrome (g, brushed, getWidth(), getHeight());
    dg::drawSpaced (g, "BUS SENDER", 292.f, 12.f, 40.f, 20.f, 2.5f, juce::Colour (0xffcfd3d8));

    for (const auto& s : sections)
        dg::drawSection (g, s.bounds, s.title, s.accent);
}

//==============================================================================
SenderEditor::SenderEditor (SenderProcessor& p)
    : juce::AudioProcessorEditor (p), panel (p)
{
    addAndMakeVisible (panel);

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) SenderPanel::baseW / (double) SenderPanel::baseH);
    setResizeLimits (SenderPanel::baseW / 2, SenderPanel::baseH / 2,
                     SenderPanel::baseW * 2, SenderPanel::baseH * 2);
    setSize (SenderPanel::baseW, SenderPanel::baseH);
}

void SenderEditor::resized()
{
    const float scale = (float) getWidth() / (float) SenderPanel::baseW;
    panel.setBounds (0, 0, SenderPanel::baseW, SenderPanel::baseH);
    panel.setTransform (juce::AffineTransform::scale (scale));
}
