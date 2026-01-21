#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination7AdaptiveFilterAlgorithm {
public:
    explicit Combination7AdaptiveFilterAlgorithm(float sampleRate)
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
        const float cutoff = param1;
        const float resonance = param2;
        const float mix = std::clamp(param3, 0.0f, 1.0f);

        const float dsfDecay = 0.5f + resonance * 0.3f;
        phase = stepPhase(phase, pitch, sampleRate);
        const float theta = TWO_PI * (1.0f + cutoff * 2.0f);
        const float denom = 1.0f - 2.0f * dsfDecay * std::cos(theta) + dsfDecay * dsfDecay;
        const float dsf = clampAbs((std::sin(TWO_PI * phase) - dsfDecay * std::sin(TWO_PI * phase - theta))
            / (denom + EPSILON), 1.0f);

        // Prev tune: DSF + sine mix, raw *0.8, clipAmount 0.4, slewCoeff 0.06, limit 1.0.
        modPhase = stepPhase(modPhase, pitch, sampleRate);
        const float mod = std::sin(TWO_PI * modPhase) * 0.5f;
        const float rawPrimary = (dsf * (1.0f - mix) + mod * mix) * 0.8f;
        const float rawSecondary = dsf * 0.8f;
        const float clipAmount = 0.4f;
        const float slewCoeff = 0.06f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.9f);
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
