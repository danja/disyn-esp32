#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class DirichletPulseAlgorithm {
public:
    explicit DirichletPulseAlgorithm(float sampleRate)
        : sampleRate(sampleRate), phase(0.0f), outPrimary(0.0f), outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        // Prev tune: harmonics up to 32, tilt -6..+6 dB, denom min 1e-2, limit base 1.0.
        const int harmonics = std::max(1, static_cast<int>(std::round(1.0f + param1 * 31.0f)));
        const float tilt = -6.0f + param2 * 12.0f;
        const float shape = std::clamp(param3, 0.0f, 1.0f);

        phase = stepPhase(phase, pitch, sampleRate);
        const float theta = phase * TWO_PI;

        const float numerator = std::sin((2.0f * harmonics + 1.0f) * theta * 0.5f);
        const float denominator = std::sin(theta * 0.5f);

        float value;
        if (std::abs(denominator) < 1e-2f) {
            const float safeDenom = (denominator < 0.0f ? -1e-2f : 1e-2f);
            value = (numerator / safeDenom) - 1.0f;
        } else {
            value = (numerator / denominator) - 1.0f;
        }

        const float tiltFactor = std::pow(10.0f, tilt / 20.0f);
        const float base = (value / static_cast<float>(harmonics)) * tiltFactor;
        const float limitedBase = clampAbs(base, 1.0f);
        const float shaped = std::tanh(limitedBase * (1.0f + shape * 4.0f));
        const float rawPrimary = limitedBase * (1.0f - shape) + shaped * shape;
        const float rawSecondary = limitedBase;
        const float clipAmount = 0.7f;
        const float slewCoeff = 0.05f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.4f);
    }

private:
    float sampleRate;
    float phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
