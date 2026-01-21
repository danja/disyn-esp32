#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination4FeedbackAlgorithm {
public:
    explicit Combination4FeedbackAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          modPhase(0.0f),
          feedbackSample(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        modPhase = 0.0f;
        feedbackSample = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        // Prev tune: simplified tanh carrier, clipAmount 0.6, slewCoeff 0.05, limit 0.6.
        const float drive = 0.6f + std::clamp(param1, 0.0f, 1.0f) * 2.0f;
        const float modifiedFreq = pitch;

        phase = stepPhase(phase, modifiedFreq, sampleRate);
        modPhase = stepPhase(modPhase, modifiedFreq, sampleRate);
        const float carrier = std::cos(TWO_PI * phase);
        feedbackSample = 0.0f;

        const float shaped = std::tanh(carrier * drive);
        const float rawPrimary = shaped * 0.6f;
        const float rawSecondary = carrier * 0.4f;
        const float clipAmount = 0.5f;
        const float slewCoeff = 0.06f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.8f);
    }

private:
    float sampleRate;
    float phase;
    float modPhase;
    float feedbackSample;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
