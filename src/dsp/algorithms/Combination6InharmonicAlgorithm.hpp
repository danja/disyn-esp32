#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination6InharmonicAlgorithm {
public:
    explicit Combination6InharmonicAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          formant1Phase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        formant1Phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param1;
        const float pafShift = expoMap(param2, 5.0f, 25.0f);
        const float mix = std::clamp(param3, 0.0f, 1.0f);

        phase = stepPhase(phase, pitch, sampleRate);
        const float dsf = std::sin(TWO_PI * phase) * 0.5f;

        const float formantFreq = pitch * 2.0f + pafShift;
        formant1Phase = stepPhase(formant1Phase, formantFreq, sampleRate);
        const float paf = std::sin(TWO_PI * formant1Phase) * 0.5f;

        // Prev tune: limitedMix 0.4, rawSecondary *0.6, clipAmount 0.8, slewCoeff 0.05, limit 0.5.
        const float limitedMix = mix * 0.3f;
        const float rawPrimary = dsf * (1.0f - limitedMix) + paf * limitedMix;
        const float rawSecondary = dsf * 0.5f;
        const float clipAmount = 0.9f;
        const float slewCoeff = 0.04f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.4f);
    }

private:
    float sampleRate;
    float phase;
    float formant1Phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
