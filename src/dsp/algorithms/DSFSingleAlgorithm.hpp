#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class DSFSingleAlgorithm {
public:
    explicit DSFSingleAlgorithm(float sampleRate)
        : sampleRate(sampleRate), phase(0.0f), secondaryPhase(0.0f), outPrimary(0.0f), outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        secondaryPhase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        // Prev tune: sine blend, ratio max 2.0, clipAmount 0.6, slewCoeff 0.06, limit 0.8.
        const float ratio = expoMap(param2, 0.5f, 1.5f);
        const float mix = std::clamp(param3, 0.0f, 1.0f);

        phase = stepPhase(phase, pitch, sampleRate);
        secondaryPhase = stepPhase(secondaryPhase, pitch * ratio, sampleRate);

        const float w = phase * TWO_PI;
        const float t = secondaryPhase * TWO_PI;

        const float carrier = std::sin(w) * 0.4f;
        const float mod = std::sin(t) * 0.4f;
        const float rawPrimary = carrier * (1.0f - mix) + mod * mix;
        const float rawSecondary = carrier * 0.5f;
        const float clipAmount = 0.7f;
        const float slewCoeff = 0.05f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.6f);
    }

private:
    float sampleRate;
    float phase;
    float secondaryPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
