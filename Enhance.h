#pragma once
#include <juce_dsp/juce_dsp.h>
#include "DspUtils.h"

namespace dg
{
// Camino paralelo de coloración armónica pura (sin compresión):
//  - Graves (~150 Hz): armónicos pares que dan cuerpo (300-600 Hz) sin tocar el subgrave.
//  - Agudos (~5 kHz): armónicos sedosos (10 kHz y más) que dan aire.
// Cada banda se satura y después se filtra para quedarse SOLO con las armónicas nuevas
// (se elimina el residuo de la fundamental). Salida: dry + wet * armónicas.
class Enhance
{
public:
    void prepare (double sampleRate, int maxBlockSize)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };

        prepBand (bassIn,  spec, juce::dsp::StateVariableTPTFilterType::bandpass, 150.0f, 0.8f);
        prepBand (trebleIn, spec, juce::dsp::StateVariableTPTFilterType::bandpass, 5000.0f, 0.7f);
        prepBand (bassOut,  spec, juce::dsp::StateVariableTPTFilterType::highpass, 220.0f, 0.707f);
        prepBand (trebleOut, spec, juce::dsp::StateVariableTPTFilterType::highpass, 7000.0f, 0.707f);

        wetSm.reset (sampleRate, 0.03);
        wetSm.setCurrentAndTargetValue (0.f);
        amtSm.reset (sampleRate, 0.03);
        amtSm.setCurrentAndTargetValue (0.f);
        reset();
    }

    void reset()
    {
        bassIn.reset(); trebleIn.reset();
        bassOut.reset(); trebleOut.reset();
    }

    // wet: 0..1 (mezcla), amount: 0..1 (cuánta saturación/armónicas se generan)
    void process (juce::AudioBuffer<float>& buf, int n, float wet, float amount)
    {
        wetSm.setTargetValue (wet);
        amtSm.setTargetValue (amount);
        float* data[2] = { buf.getWritePointer (0), buf.getWritePointer (1) };

        for (int s = 0; s < n; ++s)
        {
            const float w = wetSm.getNextValue();
            const float a = amtSm.getNextValue();

            const float gBass   = 2.0f + 18.0f * a;     // 2 .. 20
            const float gTreble = 0.5f * gBass;         // menos en agudos (evita aliasing)
            const float trim    = juce::Decibels::decibelsToGain (12.0f * a);

            for (int ch = 0; ch < 2; ++ch)
            {
                const float x = data[ch][s];
                const float b = bassIn.processSample (ch, x);
                const float t = trebleIn.processSample (ch, x);

                // Residuo no lineal normalizado (sin la parte lineal) y filtrado a las armónicas
                float hb = asymTanh (gBass * b, biasBass) / (slopeBass * gBass) - b;
                float ht = asymTanh (gTreble * t, biasTreble) / (slopeTreble * gTreble) - t;

                hb = bassOut.processSample (ch, hb);
                ht = trebleOut.processSample (ch, ht);

                data[ch][s] = x + w * trim * (hb + ht);
            }
        }
    }

private:
    using SVF = juce::dsp::StateVariableTPTFilter<float>;

    static void prepBand (SVF& f, const juce::dsp::ProcessSpec& spec,
                          juce::dsp::StateVariableTPTFilterType type, float freq, float q)
    {
        f.prepare (spec);
        f.setType (type);
        f.setCutoffFrequency (freq);
        f.setResonance (q);
    }

    SVF bassIn, trebleIn, bassOut, trebleOut;
    juce::SmoothedValue<float> wetSm, amtSm;

    static constexpr float biasBass = 0.30f, biasTreble = 0.20f;
    const float slopeBass = asymSlope (biasBass);
    const float slopeTreble = asymSlope (biasTreble);
};
} // namespace dg
