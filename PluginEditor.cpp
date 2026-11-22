#include "PluginEditor.h"

using dg::pal::sand;
using dg::pal::copper;
using dg::pal::teal;
using dg::pal::gold;
using dg::pal::blue;
using dg::pal::peri;

//==============================================================================
MasterPanel::MasterPanel (MasterBusProcessor& p)
    : proc (p),
      grMeter ("GAIN REDUCTION  (dB)", 0.f, 20.f, { 0.f, 2.f, 4.f, 6.f, 8.f, 10.f, 15.f, 20.f }, 12.f),
      vuMeter ("OUTPUT VU  (0 VU = -18 dBFS)", -20.f, 3.f, { -20.f, -10.f, -7.f, -5.f, -3.f, 0.f, 3.f }, 0.f),
      kInTrim    (p.apvts, "inTrim",       "IN TRIM",   sand,   "dB"),
      kOutTrim   (p.apvts, "outTrim",      "OUT TRIM",  sand,   "dB"),
      kRecovery  (p.apvts, "recovery",     "RECOVERY",  copper, "dB"),
      kTolerance (p.apvts, "tolerance",    "TOLERANCE", copper),
      kCrosstalk (p.apvts, "crosstalk",    "CROSSTALK", copper),
      kDriveMode (p.apvts, "driveMode",    "MODE",      teal,   "", { "DISC", "TAPE", "GRIT", "MCL" }),
      kDrive     (p.apvts, "drive",        "DRIVE",     teal,   "dB"),
      kIron      (p.apvts, "ironStrength", "IRON",      teal),
      kMorph     (p.apvts, "ironMorph",    "MORPH",     teal),
      kThickAmt  (p.apvts, "enhAmount",    "AMOUNT",    gold),
      kMix       (p.apvts, "wetDry",       "MIX",       gold,   "%"),
      kThreshold (p.apvts, "threshold",    "THRESHOLD", blue,   "dB"),
      kRatio     (p.apvts, "ratio",        "RATIO",     blue,   "", { "2", "4", "10" }),
      kAttack    (p.apvts, "attack",       "ATTACK",    blue,   "", { "0.1", "0.3", "1", "3", "10", "30" }),
      kRelease   (p.apvts, "release",      "RELEASE",   blue,   "", { "0.1", "0.3", "0.6", "1.2", "A" }),
      kMakeup    (p.apvts, "makeup",       "MAKEUP",    blue,   "dB"),
      kScHpf     (p.apvts, "scHpf",        "SC HPF",    blue,   "", { "OFF", "60", "90", "120" }),
      kVca       (p.apvts, "vcaColor",     "VCA COLOR", blue),
      kSpace     (p.apvts, "space",        "SPACE",     peri),
      kMono      (p.apvts, "monoFreq",     "MONO MAKER", peri)
{
    setLookAndFeel (&laf);
    brushed = dg::makeBrushed (baseW, baseH);

    kDriveMode.displayOverride = [] (double v)
    {
        static const char* names[] = { "CLASS A", "TAPE", "GRIT", "BUS DRVR" };
        return juce::String (names[juce::jlimit (0, 3, juce::roundToInt (v))]);
    };

    kDrive.displayOverride = [this] (double v)
    {
        const bool busDriver = (int) proc.apvts.getRawParameterValue ("driveMode")->load() == 3;
        return busDriver ? juce::String ((int) (std::round (v / 4.0) * 4.0)) + " dB"
                         : juce::String (v, 1) + " dB";
    };

    for (auto* k : { &kInTrim, &kOutTrim, &kRecovery, &kTolerance, &kCrosstalk, &kDriveMode, &kDrive, &kIron,
                     &kMorph, &kThickAmt, &kMix, &kThreshold, &kRatio, &kAttack, &kRelease, &kMakeup,
                     &kScHpf, &kVca, &kSpace, &kMono })
        addAndMakeVisible (*k);

    bInPad.init  (*this, p.apvts, "inPad",      "IN PAD",       teal);
    bBoost.init  (*this, p.apvts, "boost",      "BOOST",        dg::pal::danger);
    bOutPad.init (*this, p.apvts, "outPad",     "OUT PAD",      teal);
    bMatch.init  (*this, p.apvts, "levelMatch", "LEVEL MATCH",  teal);
    bThick.init  (*this, p.apvts, "enhance",    "ON",           gold);
    bComp.init   (*this, p.apvts, "compOn",     "ON",           blue);

    addAndMakeVisible (grMeter);
    addAndMakeVisible (vuMeter);

    for (auto* l : { &grReadout, &vuReadout })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::textColourId, dg::pal::amber);
        l->setColour (juce::Label::backgroundColourId, juce::Colour (0xff0b0d0e));
        l->setFont (juce::FontOptions (13.f, juce::Font::bold));
        addAndMakeVisible (*l);
    }

    setSize (baseW, baseH);
    startTimerHz (30);
}

MasterPanel::~MasterPanel()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

//==============================================================================
static void placeKnob (dg::Knob& k, int px, int py)
{
    k.setBounds (px, py, 80, 126);
}

void MasterPanel::addSec (const char* t, juce::Colour c, int sx, int sy, int sw)
{
    Section s;
    s.title = t;
    s.accent = c;
    s.bounds = juce::Rectangle<int> (sx, sy, sw, 176);
    sections.push_back (s);
}

void MasterPanel::resized()
{
    sections.clear();

    const int yA = 72, yB = 258, ky = 34;
    auto addSection = [this] (const char* t, juce::Colour c, int sx, int sy, int sw) { addSec (t, c, sx, sy, sw); };
    auto place = [] (dg::Knob& k, int px, int py) { placeKnob (k, px, py); };

    int x = 20;

    addSection ("INPUT", sand, x, yA, 104);
    place (kInTrim, x + 12, yA + ky);
    x += 112;

    addSection ("MINI-BUS", copper, x, yA, 264);
    place (kRecovery,  x + 12,  yA + ky);
    place (kTolerance, x + 92,  yA + ky);
    place (kCrosstalk, x + 172, yA + ky);
    x += 272;

    addSection ("BUS DRIVER", teal, x, yA, 428);
    place (kDriveMode, x + 12,  yA + ky);
    place (kDrive,     x + 92,  yA + ky);
    place (kIron,      x + 172, yA + ky);
    place (kMorph,     x + 252, yA + ky);
    {
        const int bx = x + 340;
        bInPad.button.setBounds  (bx, yA + 38,  76, 26);
        bBoost.button.setBounds  (bx, yA + 70,  76, 26);
        bOutPad.button.setBounds (bx, yA + 102, 76, 26);
        bMatch.button.setBounds  (bx, yA + 134, 76, 26);
    }
    x += 436;

    addSection ("THICKNESS", gold, x, yA, 256);
    place (kThickAmt, x + 12, yA + ky);
    place (kMix,      x + 92, yA + ky);
    bThick.button.setBounds (x + 180, yA + 66, 64, 52);

    x = 20;
    addSection ("BUS COMPRESSOR", blue, x, yB, 656);
    bComp.button.setBounds (x + 12, yB + 66, 64, 52);
    place (kThreshold, x + 84,  yB + ky);
    place (kRatio,     x + 164, yB + ky);
    place (kAttack,    x + 244, yB + ky);
    place (kRelease,   x + 324, yB + ky);
    place (kMakeup,    x + 404, yB + ky);
    place (kScHpf,     x + 484, yB + ky);
    place (kVca,       x + 564, yB + ky);
    x += 664;

    addSection ("STEREO", peri, x, yB, 184);
    place (kSpace, x + 12, yB + ky);
    place (kMono,  x + 92, yB + ky);
    x += 192;

    addSection ("OUTPUT", sand, x, yB, 104);
    place (kOutTrim, x + 12, yB + ky);
    x += 112;

    plateRect = juce::Rectangle<int> (x, yB, 108, 176);
    flowRect = juce::Rectangle<int> (480, 15, 910, 32);

    grMeter.setBounds (1108, yA, 292, 146);
    grReadout.setBounds (1108, yA + 150, 292, 24);
    vuMeter.setBounds (1108, yB, 292, 146);
    vuReadout.setBounds (1108, yB + 150, 292, 24);
}

void MasterPanel::paint (juce::Graphics& g)
{
    dg::drawChrome (g, brushed, getWidth(), getHeight());
    dg::drawSpaced (g, "MASTER BUS", 292.f, 12.f, 40.f, 20.f, 2.5f, juce::Colour (0xffcfd3d8));

    for (const auto& s : sections)
        dg::drawSection (g, s.bounds, s.title, s.accent);

    {
        const auto fr = flowRect.toFloat();
        g.setColour (juce::Colour (0xff0b0c0e));
        g.fillRoundedRectangle (fr, 6.f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (fr, 6.f, 1.f);

        static const char* names[] = { "INPUT", "MINI-BUS", "BUS DRIVER", "THICKNESS", "COMP", "STEREO", "OUTPUT" };
        const float slot = fr.getWidth() / 7.f;
        g.setFont (juce::FontOptions (11.f, juce::Font::bold));

        for (int i = 0; i < 7; ++i)
        {
            const juce::Rectangle<float> sr (fr.getX() + slot * (float) i, fr.getY(), slot, fr.getHeight());
            g.setColour (flowOn[(size_t) i] ? dg::pal::amber : juce::Colour (0xff565a60));
            g.drawText (names[i], sr, juce::Justification::centred, false);

            if (i < 6)
            {
                const float cx = sr.getRight();
                const float cy = sr.getCentreY();
                juce::Path chevron;
                chevron.addTriangle (cx - 3.f, cy - 4.f, cx - 3.f, cy + 4.f, cx + 3.f, cy);
                g.setColour (juce::Colour (0xff6b6f75));
                g.fillPath (chevron);
            }
        }
    }

    {
        const auto pr = plateRect.toFloat();
        g.setColour (juce::Colour (0xff17191c));
        g.fillRoundedRectangle (pr, 8.f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (pr, 8.f, 1.f);

        g.setColour (juce::Colour (0xffd9dce0));
        g.setFont (juce::FontOptions (12.f, juce::Font::bold));
        g.drawText ("DG-MASTERBUS", pr.withTrimmedTop (46.f).withHeight (18.f), juce::Justification::centred, false);
        g.setColour (juce::Colour (0xff8b9097));
        g.setFont (juce::FontOptions (9.f, juce::Font::bold));
        g.drawText ("ANALOG BUS", pr.withTrimmedTop (68.f).withHeight (12.f), juce::Justification::centred, false);
        g.drawText ("PROCESSOR", pr.withTrimmedTop (80.f).withHeight (12.f), juce::Justification::centred, false);
        g.setColour (dg::pal::amber);
        g.drawText ("VST3  v0.3", pr.withTrimmedTop (104.f).withHeight (12.f), juce::Justification::centred, false);
    }
}

//==============================================================================
void MasterPanel::timerCallback()
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

    grReadout.setText ("GR " + juce::String (grShown, 1) + " dB     pico " + juce::String (grPeak, 1) + " dB",
                       juce::dontSendNotification);

    const float db = juce::Decibels::gainToDecibels (proc.vuLevel.load(), -100.f);
    const float vu = db + 18.f;
    vuMeter.setValue (vu);
    vuReadout.setText (juce::String (db, 1) + " dBFS   (" + juce::String (vu, 1) + " VU)",
                       juce::dontSendNotification);

    const bool monoOn = proc.apvts.getRawParameterValue ("monoFreq")->load() > 20.5f;
    const bool spaceOn = proc.apvts.getRawParameterValue ("space")->load() > 0.001f;
    const std::array<bool, 7> now { true, true, true, bThick.button.getToggleState(),
                                    bComp.button.getToggleState(), monoOn || spaceOn, true };
    if (now != flowOn)
    {
        flowOn = now;
        repaint (flowRect);
    }

    const int mode = (int) proc.apvts.getRawParameterValue ("driveMode")->load();
    if (mode != lastMode)
    {
        lastMode = mode;
        kDrive.repaint();
    }
}

//==============================================================================
MasterEditor::MasterEditor (MasterBusProcessor& p)
    : juce::AudioProcessorEditor (p), panel (p)
{
    addAndMakeVisible (panel);

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) MasterPanel::baseW / (double) MasterPanel::baseH);
    setResizeLimits (MasterPanel::baseW / 2, MasterPanel::baseH / 2,
                     MasterPanel::baseW * 3 / 2, MasterPanel::baseH * 3 / 2);
    setSize (MasterPanel::baseW * 9 / 10, MasterPanel::baseH * 9 / 10);
}

void MasterEditor::resized()
{
    const float scale = (float) getWidth() / (float) MasterPanel::baseW;
    panel.setBounds (0, 0, MasterPanel::baseW, MasterPanel::baseH);
    panel.setTransform (juce::AffineTransform::scale (scale));
}
