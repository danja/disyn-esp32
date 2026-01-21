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
        const float decay = std::min(param1 * 0.98f, 0.98f);
        const float ratio = expoMap(param2, 0.5f, 4.0f);
        const float mix = std::clamp(param3, 0.0f, 1.0f);

        phase = stepPhase(phase, pitch, sampleRate);
        secondaryPhase = stepPhase(secondaryPhase, pitch * ratio, sampleRate);

        const float w = phase * TWO_PI;
        const float t = secondaryPhase * TWO_PI;

        const float dsf = clampAbs(computeDSFComponent(w, t, decay), 1.0f) * 0.5f;
        const float sine = std::sin(w) * 0.5f;
        const float rawPrimary = dsf * (1.0f - mix) + sine * mix;
        const float rawSecondary = dsf;
        const float clipAmount = 1.0f;
        const float slewCoeff = 0.05f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutput(smoothedPrimary, smoothedSecondary);
    }

private:
    float sampleRate;
    float phase;
    float secondaryPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
