#pragma once
#include <juce_dsp/juce_dsp.h>
#include "DspUtils.h"

namespace dg
{
// Drive con 4 modos y oversampling 4x (2 etapas half-band) para evitar aliasing.
//  0 = Discrete / Class A : tanh asimétrica (armónicos pares) + bloqueo de DC
//  1 = Tape / Smooth      : arco-tangente suave + redondeo de agudos
//  2 = Grit / Hard        : clipping duro suavizado, simétrico y denso
//  3 = Bus Driver         : modelo del McLoone Bus Driver (ver abajo)
//
// Modo 3 (Bus Driver). Escala: 1.0 = 0 dBFS = +22 dBu, el techo de salida del hardware.
//  a) Etapa de salida clase A: lineal y limpia hasta cerca del techo, limite suave y
//     ligera asimetria (bias) que aporta segundas armonicas solo al empujarla.
//  b) Transformador de salida de nucleo de niquel, modelado en el dominio del flujo magnetico:
//         v_out = v_in - G(flujo),   flujo' = v_out
//     La corriente de magnetizacion G crece muy rapido cuando el flujo se acerca al limite del
//     nucleo. Como el flujo es la integral de la tension, el nucleo satura antes en graves y
//     a nivel alto: transparente en medios y agudos, distorsion de tercera armonica en el extremo
//     grave. Tambien da la caida natural de graves (HPF de 1er orden en ~8 Hz).
//  La ganancia previa (Drive + Boost) se aplica FUERA de esta clase; aqui driveDb = 0.
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

        // Modelo del transformador
        dtOs = (float) (1.0 / osRate);
        ironA = juce::MathConstants<float>::twoPi * 8.0f; // esquina de graves (8 Hz)
        {
            const float b = ampBias;
            ampOffset = b / std::cbrt (std::sqrt (1.0f + b * b * b * b * b * b));
        }
        setIron (0.5f, 0.0f);

        driveSm.reset (osRate, 0.03);
        driveSm.setCurrentAndTargetValue (0.f);
        reset();
    }

    void reset()
    {
        if (os) os->reset();
        for (auto& d : dc) d.reset();
        lp[0] = lp[1] = 0.f;
        phi[0] = phi[1] = 0.f;
    }

    float getLatencySamples() const { return os ? os->getLatencyInSamples() : 0.f; }

    // strength: 0..1 (0.5 = nominal; mas alto = el nucleo satura antes)
    // morph:    0..1 (asimetria del flujo => mas armonicos pares)
    void setIron (float strength, float morph)
    {
        phik = 0.006366f * std::pow (10.0f, -2.0f * (strength - 0.5f));
        ironDelta = morph * 0.6f * phik;
        const float z = ironDelta / phik;
        hDelta = ironDelta - phik * z / std::sqrt (std::sqrt (1.0f + z * z * z * z));
    }

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

                    case 3:
                    {
                        // a) Etapa clase A: x / (1 + x^6)^(1/6), con bias
                        const float xa = x + ampBias;
                        const float a2 = xa * xa;
                        const float a6 = a2 * a2 * a2;
                        const float v = xa / std::cbrt (std::sqrt (1.0f + a6)) - ampOffset;

                        // b) Transformador de salida (flujo magnetico)
                        const float u = phi[ch] + ironDelta;
                        const float z = u / phik;
                        const float z2 = z * z;
                        const float h = u - phik * z / std::sqrt (std::sqrt (1.0f + z2 * z2));
                        const float vo = v - ironA * (phi[ch] + ironKappa * (h - hDelta));

                        phi[ch] += dtOs * vo;
                        if (! std::isfinite (phi[ch]))
                            phi[ch] = 0.f;

                        y = vo;
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

    // Bus Driver
    float phi[2] { 0.f, 0.f };
    float dtOs = 5.67e-6f;
    float ironA = 50.27f;
    float ironKappa = 60.0f;
    float phik = 0.006366f;
    float ironDelta = 0.f, hDelta = 0.f;
    float ampBias = 0.03f, ampOffset = 0.03f;
};
} // namespace dg
