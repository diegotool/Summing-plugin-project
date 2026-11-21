#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// Vúmetro analógico de aguja (esfera color papel, zona roja, marcas con números)
class NeedleMeter : public juce::Component
{
public:
    NeedleMeter (juce::String titleText, float minValue, float maxValue,
                 std::vector<float> tickValues, float redFrom)
        : title (std::move (titleText)), minV (minValue), maxV (maxValue),
          ticks (std::move (tickValues)), dangerFrom (redFrom), value (minValue)
    {
    }

    void setValue (float v)
    {
        v = juce::jlimit (minV, maxV, v);
        if (std::abs (v - value) > 0.01f)
        {
            value = v;
            repaint();
        }
    }

    void paint (juce::Graphics& g) override;

private:
    juce::String title;
    float minV, maxV;
    std::vector<float> ticks;
    float dangerFrom;
    float value;
};

class MasterEditor : public juce::AudioProcessorEditor,
                     private juce::Timer
{
public:
    explicit MasterEditor (MasterBusProcessor&);
    ~MasterEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void addSection (const juce::String& text);
    void addSlider (const juce::String& id, const juce::String& text);
    void addCombo (const juce::String& id, const juce::String& text);
    void addToggle (const juce::String& id, const juce::String& text);

    using SliderAtt = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAtt  = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAtt = juce::AudioProcessorValueTreeState::ButtonAttachment;

    struct SliderRow { juce::Slider slider; juce::Label label; std::unique_ptr<SliderAtt> att; };
    struct ComboRow  { juce::ComboBox box;  juce::Label label; std::unique_ptr<ComboAtt> att; };
    struct ToggleRow { juce::ToggleButton button; std::unique_ptr<ButtonAtt> att; };
    struct Row { juce::Component* label; juce::Component* control; bool isHeader; };

    MasterBusProcessor& proc;

    NeedleMeter grMeter;
    NeedleMeter vuMeter;
    juce::Label grReadout, vuReadout;

    std::vector<std::unique_ptr<juce::Label>> headers;
    std::vector<std::unique_ptr<SliderRow>> sliders;
    std::vector<std::unique_ptr<ComboRow>> combos;
    std::vector<std::unique_ptr<ToggleRow>> toggles;
    std::vector<Row> layoutRows;

    float grShown = 0.f, grPeak = 0.f;
    int grHold = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterEditor)
};
