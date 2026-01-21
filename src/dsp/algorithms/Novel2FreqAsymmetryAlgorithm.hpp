#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Novel2FreqAsymmetryAlgorithm {
public:
    explicit Novel2FreqAsymmetryAlgorithm(float sampleRate)
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
        const float lowR = 0.5f + param1 * 0.5f;
        const float highR = 1.0f + param2 * 1.0f;
        const float index = 0.2f + std::clamp(param3, 0.0f, 1.0f) * 0.8f;

        float r;
        if (pitch < 500.0f) {
            r = lowR;
        } else if (pitch > 2000.0f) {
            r = highR;
        } else {
            const float alpha = (pitch - 500.0f) / 1500.0f;
            r = lowR * (1.0f - alpha) + highR * alpha;
        }

        const float output = processAsymmetricFM(index, r / 2.0f, pitch, sampleRate, phase, modPhase);
        const float mod = std::sin(TWO_PI * modPhase);
        const float secondary = std::cos(TWO_PI * phase + index * mod) * 0.5f;
        const float clipAmount = 0.7f;
        const float slewCoeff = 0.03f;
        const float smoothedPrimary = shapeAndSlew(output, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(secondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutput(smoothedPrimary, smoothedSecondary);
    }

private:
    float sampleRate;
    float phase;
    float modPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
