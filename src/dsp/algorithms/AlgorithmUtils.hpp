#pragma once

#include <algorithm>
#include <cmath>

#include "AlgorithmOutput.hpp"

#ifdef TWO_PI
#undef TWO_PI
#endif

namespace flues::disyn
{

    constexpr float TWO_PI = 2.0f * M_PI;
    constexpr float EPSILON = 1e-8f;
    constexpr float DSF_MIN_DENOM = 1e-2f;
    constexpr float ASYM_MAX_GAIN = 1.5f;
    constexpr float OUTPUT_LIMIT = 0.5f;

    inline float stepPhase(float currentPhase, float frequency, float sampleRate)
    {
        const float next = currentPhase + frequency / sampleRate;
        return next - std::floor(next);
    }

    inline float expoMap(float value, float min, float max)
    {
        const float clamped = std::clamp(value, 0.0f, 1.0f);
        return min * std::pow(max / min, clamped);
    }

    inline float clampAbs(float value, float limit)
    {
        return std::clamp(value, -limit, limit);
    }

    inline float clampAudio(float value)
    {
        return clampAbs(value, OUTPUT_LIMIT);
    }

    inline float softClip(float value)
    {
        return std::tanh(value);
    }

    inline float softClipBlend(float value, float amount)
    {
        const float clipped = softClip(value);
        return value + (clipped - value) * std::clamp(amount, 0.0f, 1.0f);
    }

    inline float slewLimit(float value, float &state, float coeff)
    {
        state += (value - state) * coeff;
        return state;
    }

    inline float shapeAndSlew(float value, float &state, float slewCoeff, float clipAmount)
    {
        const float shaped = softClipBlend(value, clipAmount);
        return slewLimit(shaped, state, slewCoeff);
    }

    inline AlgorithmOutput normalizeOutput(float primary, float secondary)
    {
        const float maxAbs = std::max(std::abs(primary), std::abs(secondary));
        if (maxAbs > OUTPUT_LIMIT && maxAbs > 0.0f)
        {
            const float scale = OUTPUT_LIMIT / maxAbs;
            primary *= scale;
            secondary *= scale;
        }
        return {primary, secondary};
    }

    inline float safeExp(float x)
    {
        if (x > 4.0f)
        {
            x = 4.0f;
        }
        else if (x < -4.0f)
        {
            x = -4.0f;
        }
        return std::exp(x);
    }

    inline float computeDSFComponent(float w, float t, float decay)
    {
        float denominator = 1.0f - 2.0f * decay * std::cos(t) + decay * decay;
        if (denominator < DSF_MIN_DENOM)
        {
            denominator = DSF_MIN_DENOM;
        }

        const float numerator = std::sin(w) - decay * std::sin(w - t);
        const float normalise = std::sqrt(1.0f - decay * decay);
        return (numerator / denominator) * normalise;
    }

    inline float processAsymmetricFM(float param1, float param2, float frequency,
                                     float sampleRate, float &carrierPhaseRef, float &modPhaseRef)
    {
        const float k = expoMap(param1, 0.01f, 10.0f);
        const float r = expoMap(param2, 0.5f, 2.0f);
        const float modFreq = frequency;

        carrierPhaseRef = stepPhase(carrierPhaseRef, frequency, sampleRate);
        modPhaseRef = stepPhase(modPhaseRef, modFreq, sampleRate);

        const float modulator = std::sin(TWO_PI * modPhaseRef);
        float asymmetry = safeExp(k * (r - 1.0f / r) * std::cos(TWO_PI * modPhaseRef) / 2.0f);
        if (asymmetry > ASYM_MAX_GAIN)
        {
            asymmetry = ASYM_MAX_GAIN;
        }
        const float carrier = std::cos(TWO_PI * carrierPhaseRef + k * modulator);

        return carrier * asymmetry * 0.5f;
    }

    inline float wrapAngle(float x)
    {
        float wrapped = x;
        while (wrapped > M_PI)
        {
            wrapped -= TWO_PI;
        }
        while (wrapped < -M_PI)
        {
            wrapped += TWO_PI;
        }
        return wrapped;
    }

    inline float computeTaylorSine(float x, int numTerms)
    {
        const float wrapped = wrapAngle(x);

        float result = 0.0f;
        float term = wrapped;
        const float xSquared = wrapped * wrapped;

        for (int n = 0; n < numTerms; n++)
        {
            result += term;
            const float denominator = static_cast<float>((2 * n + 2) * (2 * n + 3));
            term *= -xSquared / denominator;
        }

        return std::clamp(result, -1.5f, 1.5f);
    }

} // namespace flues::disyn
