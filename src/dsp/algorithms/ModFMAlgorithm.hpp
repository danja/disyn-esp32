#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class ModFMAlgorithm {
public:
    explicit ModFMAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          modPhase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        modPhase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        // Prev tune: index max 1.5, ratio max 2.0, raw *0.6, limit 0.6.
        const float index = expoMap(param1, 0.01f, 0.8f);
        const float ratio = expoMap(param2, 0.5f, 1.5f);
        const float feedback = std::clamp(param3, 0.0f, 1.0f) * 0.4f;
        (void)feedback;

        phase = stepPhase(phase, pitch, sampleRate);
        modPhase = stepPhase(modPhase, pitch * ratio, sampleRate);

        const float carrier = std::cos(phase * TWO_PI);
        const float modPhaseRad = modPhase * TWO_PI;
        const float modulator = std::sin(modPhaseRad);
        const float fm = std::cos(phase * TWO_PI + modulator * index * 0.5f);

        // Prev tune: raw *0.6, clipAmount 0.5, slewCoeff 0.06, limit 0.6.
        const float rawPrimary = fm * 0.5f;
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
    float modPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
