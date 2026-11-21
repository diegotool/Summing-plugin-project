#pragma once
#include <juce_dsp/juce_dsp.h>

namespace dg
{
// Matriz mid/side: M = (L+R)/2, S = (L-R)/2. Con midGain = sideGain = 1 es transparente.
inline void applySpace (float& l, float& r, float midGain, float sideGain) noexcept
{
    const float m = 0.5f * (l + r);
    const float s = 0.5f * (l - r);
    l = m * midGain + s * sideGain;
    r = m * midGain - s * sideGain;
}

// Mono Maker: todo lo que queda por debajo de la frecuencia de corte pasa a mono.
// Cruce Linkwitz-Riley de 4º orden (graves + agudos suman con respuesta plana).
// freq <= 20.5 Hz => desactivado.
class MonoMaker
{
public:
    void prepare (double sampleRate, int maxBlockSize)
    {
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };
        lr.prepare (spec);
        lr.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
        lastFreq = -1.0f;
        wasActive = false;
        lr.reset();
    }

    void reset() { lr.reset(); wasActive = false; }

    void process (juce::AudioBuffer<float>& buf, int n, float freq)
    {
        if (freq <= 20.5f)
        {
            wasActive = false;
            return;
        }

        if (! wasActive)
        {
            lr.reset();
            wasActive = true;
        }

        if (freq != lastFreq)
        {
            lr.setCutoffFrequency (freq);
            lastFreq = freq;
        }

        float* L = buf.getWritePointer (0);
        float* R = buf.getWritePointer (1);

        for (int s = 0; s < n; ++s)
        {
            float lowL, highL, lowR, highR;
            lr.processSample (0, L[s], lowL, highL);
            lr.processSample (1, R[s], lowR, highR);

            const float monoLow = 0.5f * (lowL + lowR);
            L[s] = highL + monoLow;
            R[s] = highR + monoLow;
        }
    }

private:
    juce::dsp::LinkwitzRileyFilter<float> lr;
    float lastFreq = -1.0f;
    bool wasActive = false;
};
} // namespace dg
