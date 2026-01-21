#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Novel3CrossModAlgorithm {
public:
    explicit Novel3CrossModAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          modPhase(0.0f),
          secondaryPhase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        modPhase = 0.0f;
        secondaryPhase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        const float mod1Depth = param1;
        const float mod2Depth = param2;
        (void)mod2Depth;
        const float mix = std::clamp(param3, 0.0f, 1.0f);

        // Prev tune: sine crossfade, raw *0.8, clipAmount 0.6, slewCoeff 0.06, limit 0.6.
        phase = stepPhase(phase, pitch, sampleRate);
        modPhase = stepPhase(modPhase, pitch * (1.0f + mod1Depth), sampleRate);

        const float carrier = std::sin(TWO_PI * phase);
        const float mod = std::sin(TWO_PI * modPhase);

        const float rawPrimary = (carrier * (1.0f - mix) + mod * mix) * 1.4f;
        const float rawSecondary = carrier * 1.4f;
        const float clipAmount = 0.3f;
        const float slewCoeff = 0.07f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 1.2f);
    }

private:
    float sampleRate;
    float phase;
    float modPhase;
    float secondaryPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
