#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination2CascadedAlgorithm {
public:
    explicit Combination2CascadedAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          cascade1Phase(0.0f),
          cascade2Phase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        cascade1Phase = 0.0f;
        cascade2Phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        // Prev tune: simple tanh blend, raw *0.8, clipAmount 0.5, slewCoeff 0.06, limit 0.8.
        const float drive = 0.8f + std::clamp(param1, 0.0f, 1.0f) * 2.0f;
        const float mix = std::clamp(param3, 0.0f, 1.0f);

        phase = stepPhase(phase, pitch, sampleRate);
        const float carrier = std::sin(TWO_PI * phase);
        const float shaped = std::tanh(carrier * drive);

        const float rawPrimary = (carrier * (1.0f - mix) + shaped * mix) * 0.9f;
        const float rawSecondary = shaped * 0.9f;
        const float clipAmount = 0.4f;
        const float slewCoeff = 0.06f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.9f);
    }

private:
    float sampleRate;
    float phase;
    float cascade1Phase;
    float cascade2Phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
