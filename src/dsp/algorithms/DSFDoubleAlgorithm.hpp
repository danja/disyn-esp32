#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class DSFDoubleAlgorithm {
public:
    explicit DSFDoubleAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          secondaryPhase(0.0f),
          secondaryPhaseNeg(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        secondaryPhase = 0.0f;
        secondaryPhaseNeg = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        const float decay = std::min(param1 * 0.96f, 0.96f);
        const float ratio = expoMap(param2, 0.5f, 4.5f);
        const float balance = std::clamp(param3, 0.0f, 1.0f) * 2.0f - 1.0f;
        const float weightPos = 0.5f + balance * 0.5f;
        const float weightNeg = 1.0f - weightPos;

        phase = stepPhase(phase, pitch, sampleRate);
        secondaryPhase = stepPhase(secondaryPhase, pitch * ratio, sampleRate);
        secondaryPhaseNeg = stepPhase(secondaryPhaseNeg, pitch * ratio, sampleRate);

        const float w = phase * TWO_PI;
        const float tPos = secondaryPhase * TWO_PI;
        const float tNeg = -secondaryPhaseNeg * TWO_PI;

        const float positive = clampAbs(computeDSFComponent(w, tPos, decay), 1.0f);
        const float negative = clampAbs(computeDSFComponent(w, tNeg, decay), 1.0f);

        const float rawPrimary = 0.5f * (positive * weightPos + negative * weightNeg);
        const float rawSecondary = 0.5f * (positive - negative);
        const float clipAmount = 0.2f;
        const float slewCoeff = 0.2f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutput(smoothedPrimary, smoothedSecondary);
    }

private:
    float sampleRate;
    float phase;
    float secondaryPhase;
    float secondaryPhaseNeg;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
