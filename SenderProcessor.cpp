#include "SenderProcessor.h"

using juce::ParameterID;

juce::AudioProcessorValueTreeState::ParameterLayout SenderProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "bus", 1 }, "Bus",
                 StringArray { "Drums", "Guitars", "Keys", "Vocals", "FX" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "tolerance", 1 }, "Tolerance",
                 NormalisableRange<float> (0.f, 1.f, 0.01f), 0.7f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "color", 1 }, "Color",
                 NormalisableRange<float> (0.f, 12.f, 0.1f), 3.f, AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "mode", 1 }, "Color Mode",
                 StringArray { "Discrete / Class A", "Tape / Smooth", "Grit / Hard" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "trim", 1 }, "Trim",
                 NormalisableRange<float> (-12.f, 12.f, 0.1f), 0.f, AudioParameterFloatAttributes().withLabel ("dB")));

    return { p.begin(), p.end() };
}

SenderProcessor::SenderProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool SenderProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SenderProcessor::updateTolerances (int bus)
{
    // Semilla distinta por bus: desviaciones fijas y repetibles
    juce::Random rng (0x5E4D0000 + bus * 7919);
    tol[0] = rng.nextFloat() * 2.f - 1.f;
    tol[1] = rng.nextFloat() * 2.f - 1.f;
    currentBus = bus;
}

void SenderProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    saturation.prepare (sampleRate, samplesPerBlock);
    updateTolerances ((int) param ("bus"));

    for (auto& g : gainSm)
    {
        g.reset (sampleRate, 0.05);
        g.setCurrentAndTargetValue (1.0f);
    }

    setLatencySamples (juce::roundToInt (saturation.getLatencySamples()));
}

void SenderProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buffer.getNumSamples();
    if (buffer.getNumChannels() < 2 || n == 0)
        return;

    const int bus = (int) param ("bus");
    if (bus != currentBus)
        updateTolerances (bus);

    // Color (saturación ligera, oversampling 4x)
    saturation.process (buffer, n, param ("color"), (int) param ("mode"));

    // Tolerancia por canal (hasta +-3 %) y trim de salida
    const float trim = juce::Decibels::decibelsToGain (param ("trim"));
    const float tolAmt = param ("tolerance");

    for (int ch = 0; ch < 2; ++ch)
    {
        gainSm[ch].setTargetValue (trim * (1.0f + tol[ch] * 0.03f * tolAmt));
        float* d = buffer.getWritePointer (ch);
        for (int s = 0; s < n; ++s)
            d[s] *= gainSm[ch].getNextValue();
    }
}

juce::AudioProcessorEditor* SenderProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this); // GUI provisional
}

void SenderProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void SenderProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SenderProcessor();
}
