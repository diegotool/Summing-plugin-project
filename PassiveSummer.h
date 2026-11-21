#pragma once
#include <juce_core/juce_core.h>

namespace dg
{
// Emulación de un sumador pasivo de resistencias (estilo Mini-Bus 10):
//  - Cada una de las 5 parejas estéreo pasa por una red resistiva: pérdida fija de 1/5 (-14 dB).
//  - Tolerancias de componentes: desviación de ganancia fija (determinista) por canal.
//  - Crosstalk L<->R.
//  - "Gain Recovery" repone la pérdida (por defecto +13.98 dB => suma nula frente al DAW
//    cuando tolerancia y crosstalk son 0).
class PassiveSummer
{
public:
    static constexpr int numBuses = 5;

    void prepare()
    {
        juce::Random rng (0xD6C0DE);
        for (int i = 0; i < numBuses; ++i)
            for (int ch = 0; ch < 2; ++ch)
                tol[i][ch] = rng.nextFloat() * 2.f - 1.f; // -1..+1, fijo por canal
    }

    // src: 10 punteros [bus0L, bus0R, bus1L, ...]; nullptr si el bus está desactivado.
    // tolerance: 0..1 (1 => hasta +-1 % de ganancia por canal)
    // crosstalk: 0..1 (1 => -40 dB de fuga entre L y R)
    void process (const float* const* src, float* outL, float* outR, int n,
                  float recoveryDb, float tolerance, float crosstalk) const
    {
        float gL[numBuses], gR[numBuses];
        const float base = (1.0f / (float) numBuses) * juce::Decibels::decibelsToGain (recoveryDb);

        for (int i = 0; i < numBuses; ++i)
        {
            gL[i] = base * (1.f + tol[i][0] * 0.01f * tolerance);
            gR[i] = base * (1.f + tol[i][1] * 0.01f * tolerance);
        }

        const float xt = crosstalk * 0.01f;

        for (int s = 0; s < n; ++s)
        {
            float l = 0.f, r = 0.f;

            for (int i = 0; i < numBuses; ++i)
            {
                if (src[2 * i] != nullptr)     l += gL[i] * src[2 * i][s];
                if (src[2 * i + 1] != nullptr) r += gR[i] * src[2 * i + 1][s];
            }

            outL[s] = l + xt * r;
            outR[s] = r + xt * l;
        }
    }

private:
    float tol[numBuses][2] {};
};
} // namespace dg
