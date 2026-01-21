#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class PAFAlgorithm {
public:
    explicit PAFAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          secondaryPhase(0.0f),
          modPhase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        secondaryPhase = 0.0f;
        modPhase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        // Prev tune: ratio max 3.0, depth max 0.4, raw *0.6, limit 0.6.
        const float ratio = expoMap(param1, 0.5f, 2.0f);
        const float depth = 0.1f + std::clamp(param3, 0.0f, 1.0f) * 0.2f;

        phase = stepPhase(phase, pitch, sampleRate);
        secondaryPhase = stepPhase(secondaryPhase, pitch * ratio, sampleRate);

        const float carrier = std::sin(secondaryPhase * TWO_PI);
        const float mod = std::sin(phase * TWO_PI);
        modPhase = modPhase * 0.95f + mod * 0.05f;
        // Prev tune: raw *0.6, clipAmount 0.5, slewCoeff 0.06, limit 0.6.
        const float rawPrimary = carrier * ((1.0f - depth) + depth * modPhase) * 0.5f;
        const float rawSecondary = carrier * 0.5f;
        const float clipAmount = 0.6f;
        const float slewCoeff = 0.05f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.5f);
    }

private:
    float sampleRate;
    float phase;
    float secondaryPhase;
    float modPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
