#pragma once
#include <juce_dsp/juce_dsp.h>
#include "DspUtils.h"

namespace dg
{
// Compresor de bus estilo VCA (SSL G): feed-forward, detector de pico con enlace estéreo,
// rodilla suave de 2 dB, ataque/release en dominio dB, release automático dependiente
// del programa y HPF en el sidechain.
class SSLCompressor
{
public:
    struct Params
    {
        float thresholdDb = 0.f;
        float ratio = 4.f;
        float attackMs = 3.f;
        float releaseSec = 0.3f; // < 0 => Auto
        float makeupDb = 0.f;
        float hpfHz = 0.f;       // 0 => Off
        float vcaColor = 0.f;    // 0..1: distorsion del VCA (1 = ~3x la medida en un clon del G-Bus)
    };

    void prepare (double sampleRate, int maxBlockSize)
    {
        fs = (float) sampleRate;
        dcR = 1.0f - juce::MathConstants<float>::twoPi * 10.0f / fs;
        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) maxBlockSize, 2 };
        hpf.prepare (spec);
        hpf.setType (juce::dsp::StateVariableTPTFilterType::highpass);
        hpf.setResonance (0.707f);
        reset();
    }

    void reset()
    {
        hpf.reset();
        dc[0].reset();
        dc[1].reset();
        grDb = 0.f;
    }

    // Devuelve la reducción de ganancia máxima del bloque (dB, positiva)
    float process (juce::AudioBuffer<float>& buf, int n, const Params& p)
    {
        float* l = buf.getWritePointer (0);
        float* r = buf.getWritePointer (1);

        const bool useHpf = p.hpfHz > 0.f;
        if (useHpf) hpf.setCutoffFrequency (p.hpfHz);

        const float makeup = juce::Decibels::decibelsToGain (p.makeupDb);
        const float vcaAmount = p.vcaColor;
        const float atkCoef = std::exp (-1.0f / (fs * juce::jmax (p.attackMs, 0.01f) * 0.001f));
        const float fixedRelCoef = p.releaseSec > 0.f ? std::exp (-1.0f / (fs * p.releaseSec)) : 0.f;
        const float slope = 1.0f - 1.0f / p.ratio;
        constexpr float knee = 2.0f;

        float maxGr = 0.f;

        for (int s = 0; s < n; ++s)
        {
            float dl = l[s], dr = r[s];

            if (useHpf)
            {
                dl = hpf.processSample (0, dl);
                dr = hpf.processSample (1, dr);
            }

            const float lvl = juce::jmax (std::abs (dl), std::abs (dr));
            const float lvlDb = 20.0f * std::log10 (juce::jmax (lvl, 1.0e-6f));
            const float over = lvlDb - p.thresholdDb;

            float target;
            if (2.0f * over < -knee)      target = 0.f;
            else if (2.0f * over > knee)  target = slope * over;
            else { const float x = over + 0.5f * knee; target = slope * x * x / (2.0f * knee); }

            float coef;
            if (target > grDb)
                coef = atkCoef;
            else if (p.releaseSec > 0.f)
                coef = fixedRelCoef;
            else
            {
                // Auto: más reducción => release más rápido (1.2 s -> 0.15 s)
                const float t = juce::jmap (juce::jlimit (0.f, 1.f, grDb / 10.f), 1.2f, 0.15f);
                coef = std::exp (-1.0f / (fs * t));
            }

            grDb = coef * grDb + (1.0f - coef) * target;
            maxGr = juce::jmax (maxGr, grDb);

            const float gRed = juce::Decibels::decibelsToGain (-grDb);
            float yl = l[s] * gRed;
            float yr = r[s] * gRed;

            if (vcaAmount > 0.f)
            {
                // Distorsion del VCA: casi nula sin reduccion y creciente con ella
                // (medida en un clon del G-Bus: ~0.001 % sin comprimir, ~0.22 % a 12 dB de GR).
                // Segunda armonica (x^2) + tercera (x^3); el DC del termino cuadratico se elimina.
                const float k  = vcaAmount * juce::jmin (grDb, 18.f) * (1.0f / 12.0f);
                const float k2 = 0.04f * k;
                const float k3 = 0.08f * k;
                yl = dc[0].process (yl + k2 * yl * yl - k3 * yl * yl * yl, dcR);
                yr = dc[1].process (yr + k2 * yr * yr - k3 * yr * yr * yr, dcR);
            }

            l[s] = yl * makeup;
            r[s] = yr * makeup;
        }

        return maxGr;
    }

private:
    juce::dsp::StateVariableTPTFilter<float> hpf;
    float fs = 44100.f;
    float grDb = 0.f;
    DCBlocker dc[2];
    float dcR = 0.999f;
};
} // namespace dg
