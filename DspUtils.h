#pragma once
#include <cmath>

namespace dg
{
// Bloqueador de DC de un polo: y[n] = x[n] - x[n-1] + R * y[n-1]
struct DCBlocker
{
    float x1 = 0.f, y1 = 0.f;

    void reset() noexcept { x1 = y1 = 0.f; }

    inline float process (float x, float R) noexcept
    {
        const float y = x - x1 + R * y1;
        x1 = x;
        y1 = y;
        return y;
    }
};

// Tanh asimétrica (bias > 0 => armónicos pares). Resta el offset para que f(0) = 0.
inline float asymTanh (float x, float bias) noexcept
{
    return std::tanh (x + bias) - std::tanh (bias);
}

// Pendiente de asymTanh en el origen
inline float asymSlope (float bias) noexcept
{
    const float t = std::tanh (bias);
    return 1.f - t * t;
}
} // namespace dg
