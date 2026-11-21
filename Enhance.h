#pragma once
#include <juce_dsp/juce_dsp.h>
#include "DspUtils.h"

namespace dg
{
// Camino paralelo de coloración armónica pura (sin compresión):
//  - Graves (~150 Hz): armónicos pares que dan cuerpo sin tocar el subgrave.
//  - Agudos (~5 kHz): armónicos sedosos que dan aire.
// Solo se mezclan los armónicos generados: out = dry + wet * armónicos.
class Enhance
{
public:
    void prepare (double sampleRate, int maxBlockSize)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };

        bass.prepare (spec);
        bass.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        bass.setCutoffFrequency (150.0f);
        bass.setResonance (0.8f);

        treble.prepare (spec);
        treble.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
        treble.setCutoffFrequency (5000.0f);
        treble.setResonance (0.7f);

        dcR = 1.0f - juce::MathConstants<float>::twoPi * 20.0f / (float) sampleRate;

        wetSm.reset (sampleRate, 0.03);
        wetSm.setCurrentAndTargetValue (0.f);
        reset();
    }

    void reset()
    {
        bass.reset();
        treble.reset();
        for (auto& d : dcBass)   d.reset();
        for (auto& d : dcTreble) d.reset();
    }

    // wet: 0..1
    void process (juce::AudioBuffer<float>& buf, int n, float wet)
    {
        wetSm.setTargetValue (wet);
        float* data[2] = { buf.getWritePointer (0), buf.getWritePointer (1) };

        for (int s = 0; s < n; ++s)
        {
            const float w = wetSm.getNextValue();

            for (int ch = 0; ch < 2; ++ch)
            {
                const float x = data[ch][s];
                const float b = bass.processSample (ch, x);
                const float t = treble.processSample (ch, x);

                // Solo la parte no lineal (se resta la componente lineal)
                float hb = asymTanh (gainBass * b, biasBass) / gainBass - b * slopeBass;
                float ht = asymTanh (gainTreble * t, biasTreble) / gainTreble - t * slopeTreble;

                hb = dcBass[ch].process (hb, dcR);
                ht = dcTreble[ch].process (ht, dcR);

                data[ch][s] = x + w * (amountBass * hb + amountTreble * ht);
            }
        }
    }

private:
    juce::dsp::StateVariableTPTFilter<float> bass, treble;
    juce::SmoothedValue<float> wetSm;
    DCBlocker dcBass[2], dcTreble[2];
    float dcR = 0.997f;

    // Valores de partida: ajustar de oído
    static constexpr float gainBass = 4.0f,   biasBass = 0.30f,   amountBass = 1.0f;
    static constexpr float gainTreble = 4.0f, biasTreble = 0.15f, amountTreble = 1.0f;
    const float slopeBass = asymSlope (biasBass);
    const float slopeTreble = asymSlope (biasTreble);
};
} // namespace dg
