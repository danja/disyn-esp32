#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination1HybridFormantAlgorithm {
public:
    explicit Combination1HybridFormantAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          modPhase(0.0f),
          formant1Phase(0.0f),
          formant2Phase(0.0f),
          formant3Phase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        modPhase = 0.0f;
        formant1Phase = 0.0f;
        formant2Phase = 0.0f;
        formant3Phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        const float formantSpacing = 0.9f + param3 * 0.2f;

        phase = stepPhase(phase, pitch, sampleRate);
        modPhase = stepPhase(modPhase, pitch, sampleRate);
        const float carrier = std::sin(TWO_PI * phase);
        const float base = carrier * 0.4f;

        formant1Phase = stepPhase(formant1Phase, 800.0f * formantSpacing, sampleRate);
        formant2Phase = stepPhase(formant2Phase, 1200.0f * formantSpacing, sampleRate);
        formant3Phase = stepPhase(formant3Phase, 2400.0f * formantSpacing, sampleRate);

        const float formant1 = std::sin(TWO_PI * formant1Phase) * 0.5f;
        const float formant2 = std::sin(TWO_PI * formant2Phase) * 0.5f;
        const float formant3 = std::sin(TWO_PI * formant3Phase) * 0.5f;

        // Prev tune: rawPrimary *0.4, rawSecondary *0.6, clipAmount 0.6, slewCoeff 0.05, limit 0.6.
        const float rawPrimary = (base + formant1 + formant2 + formant3) * 0.6f;
        const float rawSecondary = base * 0.6f;
        const float clipAmount = 0.5f;
        const float slewCoeff = 0.05f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.8f);
    }

private:
    float sampleRate;
    float phase;
    float modPhase;
    float formant1Phase;
    float formant2Phase;
    float formant3Phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
