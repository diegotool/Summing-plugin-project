#pragma once
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "DGWidgets.h"

// Panel completo del Master (tamano base fijo; el editor lo escala)
class MasterPanel : public juce::Component,
                    private juce::Timer
{
public:
    static constexpr int baseW = 1420;
    static constexpr int baseH = 452;

    explicit MasterPanel (MasterBusProcessor&);
    ~MasterPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void addSec (const char* t, juce::Colour c, int sx, int sy, int sw);

    struct Section { juce::String title; juce::Colour accent; juce::Rectangle<int> bounds; };

    dg::DGLookAndFeel laf;
    MasterBusProcessor& proc;
    juce::Image brushed;
    std::vector<Section> sections;
    juce::Rectangle<int> flowRect, plateRect;

    dg::NeedleMeter grMeter, vuMeter;
    juce::Label grReadout, vuReadout;

    dg::Knob kInTrim, kOutTrim;
    dg::Knob kRecovery, kTolerance, kCrosstalk;
    dg::Knob kDriveMode, kDrive, kIron, kMorph;
    dg::Knob kThickAmt, kMix;
    dg::Knob kThreshold, kRatio, kAttack, kRelease, kMakeup, kScHpf, kVca;
    dg::Knob kSpace, kMono;

    dg::LedButton bInPad, bBoost, bOutPad, bMatch, bThick, bComp;

    std::array<bool, 7> flowOn {};
    float grShown = 0.f, grPeak = 0.f;
    int grHold = 0;
    int lastMode = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterPanel)
};

class MasterEditor : public juce::AudioProcessorEditor
{
public:
    explicit MasterEditor (MasterBusProcessor&);

    void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xff1a1c1f)); }
    void resized() override;

private:
    MasterPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterEditor)
};
