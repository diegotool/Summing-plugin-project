#pragma once
#include <juce_dsp/juce_dsp.h>
#include "DspUtils.h"

namespace dg
{
// Drive con 3 modos y oversampling 4x (2 etapas half-band) para evitar aliasing.
//  0 = Discrete / Class A : tanh asimétrica (armónicos pares) + bloqueo de DC
//  1 = Tape / Smooth      : arco-tangente suave + redondeo de agudos
//  2 = Grit / Hard        : clipping duro suavizado, simétrico y denso
class Saturation
{
public:
    void prepare (double sampleRate, int maxBlockSize)
    {
        os = std::make_unique<juce::dsp::Oversampling<float>> (
            2, 2, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, true);
        os->initProcessing ((size_t) maxBlockSize);

        const double osRate = sampleRate * 4.0;
        dcR = 1.0f - juce::MathConstants<float>::twoPi * 5.0f / (float) osRate;
        lpCoef = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 18000.0f / (float) osRate);

        driveSm.reset (osRate, 0.03);
        driveSm.setCurrentAndTargetValue (0.f);
        reset();
    }

    void reset()
    {
        if (os) os->reset();
        for (auto& d : dc) d.reset();
        lp[0] = lp[1] = 0.f;
    }

    float getLatencySamples() const { return os ? os->getLatencyInSamples() : 0.f; }

    void process (juce::AudioBuffer<float>& buf, int n, float driveDb, int mode)
    {
        juce::dsp::AudioBlock<float> block (buf.getArrayOfWritePointers(), 2, (size_t) n);
        auto up = os->processSamplesUp (block);

        driveSm.setTargetValue (driveDb);
        const size_t m = up.getNumSamples();

        for (size_t s = 0; s < m; ++s)
        {
            const float g = juce::Decibels::decibelsToGain (driveSm.getNextValue());
            const float comp = 1.0f / std::sqrt (g); // compensación parcial: más drive = algo más de nivel

            for (int ch = 0; ch < 2; ++ch)
            {
                float* d = up.getChannelPointer ((size_t) ch);
                const float x = d[s] * g;
                float y;

                switch (mode)
                {
                    default:
                    case 0:
                        y = dc[ch].process (asymTanh (x, 0.25f), dcR);
                        break;

                    case 1:
                        y = (2.0f / juce::MathConstants<float>::pi)
                            * std::atan (x * juce::MathConstants<float>::halfPi);
                        lp[ch] += lpCoef * (y - lp[ch]);
                        y = lp[ch];
                        break;

                    case 2:
                    {
                        const float x2 = x * x;
                        y = x / std::sqrt (std::sqrt (1.0f + x2 * x2));
                        break;
                    }
                }

                d[s] = y * comp;
            }
        }

        os->processSamplesDown (block);
    }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> os;
    juce::SmoothedValue<float> driveSm;
    DCBlocker dc[2];
    float lp[2] { 0.f, 0.f };
    float dcR = 0.999f, lpCoef = 0.5f;
};
} // namespace dg
