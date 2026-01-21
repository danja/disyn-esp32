#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Novel1MultistageAlgorithm {
public:
    explicit Novel1MultistageAlgorithm(float sampleRate)
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
        const float tanhDrive = expoMap(param1, 0.1f, 3.0f);
        const float ringCarrierMult = 0.5f + param3 * 1.5f;

        phase = stepPhase(phase, pitch, sampleRate);
        const float input = std::sin(TWO_PI * phase);

        const float stage1 = std::tanh(tanhDrive * input);
        const float stage2 = stage1;

        modPhase = stepPhase(modPhase, pitch * ringCarrierMult, sampleRate);
        const float carrier = std::sin(TWO_PI * modPhase);
        const float stage3 = stage2 * (1.0f + carrier);

        // Prev tune: raw *0.8, clipAmount 0.7, slewCoeff 0.05, limit 0.5.
        const float rawPrimary = stage3 * 1.2f;
        const float rawSecondary = stage2 * 1.2f;
        const float clipAmount = 0.5f;
        const float slewCoeff = 0.06f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 1.0f);
    }

private:
    float sampleRate;
    float phase;
    float modPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
