#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Novel4TaylorAlgorithm {
public:
    explicit Novel4TaylorAlgorithm(float sampleRate)
        : sampleRate(sampleRate), phase(0.0f), outPrimary(0.0f), outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        const int firstTerms = std::max(1, static_cast<int>(std::round(1.0f + param1 * 9.0f)));
        const int secondTerms = std::max(1, static_cast<int>(std::round(1.0f + param2 * 9.0f)));
        const float blend = std::clamp(param3, 0.0f, 1.0f);

        phase = stepPhase(phase, pitch, sampleRate);
        const float theta = phase * TWO_PI;

        const float fundamental = computeTaylorSine(theta, firstTerms);
        const float secondHarmonic = computeTaylorSine(2.0f * theta, secondTerms);

        const float output = fundamental * (1.0f - blend) + secondHarmonic * blend;
        const float clamped = std::clamp(output, -1.0f, 1.0f);
        const float secondary = std::clamp(secondHarmonic, -1.0f, 1.0f);
        // Prev tune: clipAmount 0.8, slewCoeff 0.06, limit 0.8.
        const float clipAmount = 0.9f;
        const float slewCoeff = 0.05f;
        const float smoothedPrimary = shapeAndSlew(clamped, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(secondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.6f);
    }

private:
    float sampleRate;
    float phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
