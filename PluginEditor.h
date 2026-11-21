#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include <array>
#include <vector>

//==============================================================================
class NeedleMeter : public juce::Component
{
public:
    NeedleMeter (const juce::String& titleText,
                 float minVal,
                 float maxVal,
                 std::initializer_list<float> tickValues,
                 float dangerStart);

    void setValue (float newValue);
    void paint (juce::Graphics& g) override;

private:
    juce::String title;
    float minV = 0.f;
    float maxV = 20.f;
    std::vector<float> ticks;
    float dangerFrom = 12.f;
    float value = 0.f;
};

//==============================================================================
class SslFusionLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SslFusionLookAndFeel();

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;
};

//==============================================================================
class MasterEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit MasterEditor (MasterBusProcessor&);
    ~MasterEditor() override;

    void paint (juce::Graphics& g) override;
    void paintOverChildren (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    struct RackModule
    {
        juce::String title;
        juce::Colour accent;
        juce::Rectangle<int> bounds;
    };

    struct KnobSlot
    {
        juce::String paramId;
        juce::String labelText;
        juce::Colour accent;
        int moduleIndex = 0;
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct ButtonSlot
    {
        juce::String paramId;
        juce::String labelText;
        int moduleIndex = 0;
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    struct ComboSlot
    {
        juce::String paramId;
        juce::String labelText;
        juce::Colour accent;
        int moduleIndex = 0;
        juce::ComboBox box;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    void addKnob (int modIdx, const juce::String& id, const juce::String& text, juce::Colour accent);
    void addButton (int modIdx, const juce::String& id, const juce::String& text);
    void addCombo (int modIdx, const juce::String& id, const juce::String& text, juce::Colour accent);
    void drawBrushedGraphite (juce::Graphics& g, juce::Rectangle<float> area);
    void drawRackEars (juce::Graphics& g, juce::Rectangle<float> area);

    MasterBusProcessor& proc;
    SslFusionLookAndFeel lnf;

    juce::ComponentBoundsConstrainer constrainer;
    juce::ResizableCornerComponent resizer { this, &constrainer };

    std::array<RackModule, 5> modules;
    std::vector<std::unique_ptr<KnobSlot>> knobs;
    std::vector<std::unique_ptr<ButtonSlot>> buttons;
    std::vector<std::unique_ptr<ComboSlot>> combos;

    NeedleMeter grMeter;
    NeedleMeter vuMeter;
    juce::Label grReadout;
    juce::Label vuReadout;

    float grShown = 0.f;
    float grPeak  = 0.f;
    int   grHold  = 0;

    static constexpr int baseWidth  = 1380;
    static constexpr int baseHeight = 560;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterEditor)
};
