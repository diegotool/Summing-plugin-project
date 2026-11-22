#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "SenderProcessor.h"
#include "DGWidgets.h"

// Panel del Sender (tamano base fijo; el editor lo escala)
class SenderPanel : public juce::Component
{
public:
    static constexpr int baseW = 640;
    static constexpr int baseH = 270;

    explicit SenderPanel (SenderProcessor&);
    ~SenderPanel() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    struct Section { juce::String title; juce::Colour accent; juce::Rectangle<int> bounds; };

    dg::DGLookAndFeel laf;
    juce::Image brushed;
    std::vector<Section> sections;

    dg::Knob kBus, kTol, kColor, kMode, kWidth, kTrim;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SenderPanel)
};

class SenderEditor : public juce::AudioProcessorEditor
{
public:
    explicit SenderEditor (SenderProcessor&);

    void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xff1a1c1f)); }
    void resized() override;

private:
    SenderPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SenderEditor)
};
