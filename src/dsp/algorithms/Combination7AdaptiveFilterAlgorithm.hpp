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

        const float dsfDecay = 0.5f + resonance * 0.49f;
        phase = stepPhase(phase, pitch, sampleRate);
        const float theta = TWO_PI * (1.0f + cutoff * 2.0f);
        const float denom = 1.0f - 2.0f * dsfDecay * std::cos(theta) + dsfDecay * dsfDecay;
        const float dsf = clampAbs((std::sin(TWO_PI * phase) - dsfDecay * std::sin(TWO_PI * phase - theta))
            / (denom + EPSILON), 1.0f);

        const float modfmIndex = expoMap(cutoff, 0.01f, 2.0f);
        modPhase = stepPhase(modPhase, pitch, sampleRate);
        secondaryPhase = stepPhase(secondaryPhase, pitch, sampleRate);
        const float mod = std::cos(TWO_PI * secondaryPhase);
        const float modfm = std::cos(TWO_PI * modPhase) * safeExp(modfmIndex * (mod - 1.0f));

        const float rawPrimary = (dsf * (1.0f - mix) + modfm * mix) * 0.3f;
        const float rawSecondary = modfm * 0.3f;
        const float clipAmount = 0.7f;
        const float slewCoeff = 0.03f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutput(smoothedPrimary, smoothedSecondary);
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
