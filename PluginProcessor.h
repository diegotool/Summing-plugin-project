#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PassiveSummer.h"
#include "Saturation.h"
#include "Enhance.h"
#include "SSLCompressor.h"

class MasterBusProcessor : public juce::AudioProcessor
{
public:
    MasterBusProcessor();
    ~MasterBusProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DG-MasterBus"; }
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
    std::atomic<float> grMeterDb { 0.f }; // reducción de ganancia (dB, positiva) para el medidor GR
    std::atomic<float> vuLevel { 0.f };   // nivel de salida (lineal, balística VU) para el medidor VU

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    static BusesProperties makeBuses();

    float param (const char* id) const { return apvts.getRawParameterValue (id)->load(); }

    dg::PassiveSummer summer;
    dg::Saturation saturation;
    dg::Enhance enhance;
    dg::SSLCompressor compressor;

    juce::AudioBuffer<float> sumBuf;
    double sr = 44100.0;
    float vuEnv = 0.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasterBusProcessor)
};
