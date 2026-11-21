#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Saturation.h"
#include "StereoTools.h"

// DG-Sender: colorea cada bus antes de llegar al DG-MasterBus.
//  - Tolerance: pequeña variación de ganancia propia de cada canal L/R.
//    El selector "Bus" cambia la semilla, así cada bus tiene desviaciones distintas y fijas.
//  - Color: saturación ligera (mismos 3 modos que el Drive del Master, con oversampling 4x).
//  - Width: ancho estereo del bus (mid/side, 100 % = sin cambio) para separar los buses entre si.
//  - Trim: nivel de salida.
class SenderProcessor : public juce::AudioProcessor
{
public:
    SenderProcessor();
    ~SenderProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DG-Sender"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    float param (const char* id) const { return apvts.getRawParameterValue (id)->load(); }
    void updateTolerances (int bus);

    dg::Saturation saturation;
    juce::SmoothedValue<float> gainSm[2];
    juce::SmoothedValue<float> sideSm;
    float tol[2] { 0.f, 0.f };
    int currentBus = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SenderProcessor)
};
