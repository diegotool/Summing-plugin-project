#include "PluginProcessor.h"

using juce::ParameterID;

juce::AudioProcessor::BusesProperties MasterBusProcessor::makeBuses()
{
    const auto st = juce::AudioChannelSet::stereo();
    return BusesProperties()
        .withInput ("Drums",   st, true)
        .withInput ("Guitars", st, true)
        .withInput ("Keys",    st, true)
        .withInput ("Vocals",  st, true)
        .withInput ("FX",      st, true)
        .withOutput ("Master", st, true);
}

juce::AudioProcessorValueTreeState::ParameterLayout MasterBusProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    // Suma pasiva
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "recovery", 1 }, "Gain Recovery",
                 NormalisableRange<float> (0.f, 24.f, 0.01f), 13.98f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "tolerance", 1 }, "Tolerance",
                 NormalisableRange<float> (0.f, 1.f, 0.01f), 0.5f));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "crosstalk", 1 }, "Crosstalk",
                 NormalisableRange<float> (0.f, 1.f, 0.01f), 0.5f));

    // Drive
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "drive", 1 }, "Drive",
                 NormalisableRange<float> (0.f, 24.f, 0.1f), 6.f));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "driveMode", 1 }, "Drive Mode",
                 StringArray { "Discrete / Class A", "Tape / Smooth", "Grit / Hard" }, 0));

    // Enhance
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "enhance", 1 }, "Enhance", false));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "wetDry", 1 }, "Wet / Dry",
                 NormalisableRange<float> (0.f, 100.f, 0.1f), 30.f, AudioParameterFloatAttributes().withLabel ("%")));

    // Compresor SSL G
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "compOn", 1 }, "Comp On", true));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "threshold", 1 }, "Threshold",
                 NormalisableRange<float> (-20.f, 20.f, 0.1f), 0.f, AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "ratio", 1 }, "Ratio",
                 StringArray { "2:1", "4:1", "10:1" }, 1));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "attack", 1 }, "Attack",
                 StringArray { "0.1 ms", "0.3 ms", "1 ms", "3 ms", "10 ms", "30 ms" }, 3));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "release", 1 }, "Release",
                 StringArray { "0.1 s", "0.3 s", "0.6 s", "1.2 s", "Auto" }, 1));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "makeup", 1 }, "Makeup",
                 NormalisableRange<float> (0.f, 20.f, 0.1f), 0.f, AudioParameterFloatAttributes().withLabel ("dB")));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "scHpf", 1 }, "SC HPF",
                 StringArray { "Off", "60 Hz", "90 Hz", "120 Hz" }, 1));

    return { p.begin(), p.end() };
}

MasterBusProcessor::MasterBusProcessor()
    : AudioProcessor (makeBuses()),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool MasterBusProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto st = juce::AudioChannelSet::stereo();

    if (layouts.getMainOutputChannelSet() != st)
        return false;

    for (int i = 0; i < dg::PassiveSummer::numBuses; ++i)
    {
        const auto s = layouts.getChannelSet (true, i);
        if (! s.isDisabled() && s != st)
            return false;
    }
    return true;
}

void MasterBusProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    summer.prepare();
    saturation.prepare (sampleRate, samplesPerBlock);
    enhance.prepare (sampleRate, samplesPerBlock);
    compressor.prepare (sampleRate, samplesPerBlock);
    sumBuf.setSize (2, samplesPerBlock);

    setLatencySamples (juce::roundToInt (saturation.getLatencySamples()));
}

void MasterBusProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();

    if (n > sumBuf.getNumSamples())
        sumBuf.setSize (2, n, false, false, true); // solo si el host excede el tamaño declarado

    // 1) Punteros de los 5 buses estéreo (nullptr si el bus está desactivado)
    const float* src[2 * dg::PassiveSummer::numBuses] {};

    for (int i = 0; i < dg::PassiveSummer::numBuses; ++i)
    {
        if (auto* bus = getBus (true, i); bus != nullptr && bus->isEnabled())
        {
            auto bb = getBusBuffer (buffer, true, i);
            if (bb.getNumChannels() >= 2)
            {
                src[2 * i]     = bb.getReadPointer (0);
                src[2 * i + 1] = bb.getReadPointer (1);
            }
        }
    }

    // Etapa 1: suma pasiva
    summer.process (src, sumBuf.getWritePointer (0), sumBuf.getWritePointer (1), n,
                    param ("recovery"), param ("tolerance"), param ("crosstalk"));

    // Etapa 2: Drive (3 modos, oversampling 4x)
    saturation.process (sumBuf, n, param ("drive"), (int) param ("driveMode"));

    // Etapa 3: Enhance paralelo + Wet/Dry
    if (param ("enhance") > 0.5f)
        enhance.process (sumBuf, n, param ("wetDry") * 0.01f);

    // Etapa 4: compresor VCA estilo SSL G
    if (param ("compOn") > 0.5f)
    {
        static constexpr float ratios[]   = { 2.f, 4.f, 10.f };
        static constexpr float attacks[]  = { 0.1f, 0.3f, 1.f, 3.f, 10.f, 30.f };
        static constexpr float releases[] = { 0.1f, 0.3f, 0.6f, 1.2f, -1.f };
        static constexpr float hpfs[]     = { 0.f, 60.f, 90.f, 120.f };

        dg::SSLCompressor::Params cp;
        cp.thresholdDb = param ("threshold");
        cp.ratio       = ratios[juce::jlimit (0, 2, (int) param ("ratio"))];
        cp.attackMs    = attacks[juce::jlimit (0, 5, (int) param ("attack"))];
        cp.releaseSec  = releases[juce::jlimit (0, 4, (int) param ("release"))];
        cp.makeupDb    = param ("makeup");
        cp.hpfHz       = hpfs[juce::jlimit (0, 3, (int) param ("scHpf"))];

        grMeterDb.store (compressor.process (sumBuf, n, cp));
    }
    else
    {
        grMeterDb.store (0.f);
    }

    // Salida: copiar al bus de salida y limpiar el resto de canales
    auto out = getBusBuffer (buffer, false, 0);
    out.copyFrom (0, 0, sumBuf, 0, 0, n);
    out.copyFrom (1, 0, sumBuf, 1, 0, n);

    for (int ch = 2; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, n);
}

juce::AudioProcessorEditor* MasterBusProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this); // GUI provisional (la GUI final es la Fase 3)
}

void MasterBusProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void MasterBusProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MasterBusProcessor();
}
