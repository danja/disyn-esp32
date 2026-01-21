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
        const float lowR = 0.8f + param1 * 0.2f;
        const float highR = 1.0f + param2 * 0.2f;
        const float index = 0.1f + std::clamp(param3, 0.0f, 1.0f) * 0.3f;

        float r;
        if (pitch < 500.0f) {
            r = lowR;
        } else if (pitch > 2000.0f) {
            r = highR;
        } else {
            const float alpha = (pitch - 500.0f) / 1500.0f;
            r = lowR * (1.0f - alpha) + highR * alpha;
        }

        modPhase = stepPhase(modPhase, pitch * r, sampleRate);
        phase = stepPhase(phase, pitch, sampleRate);
        const float mod = std::sin(TWO_PI * modPhase);
        const float output = std::cos(TWO_PI * phase + mod * index) * 0.4f;
        const float secondary = std::cos(TWO_PI * phase) * 0.4f;
        // Prev tune: output *1.0, clipAmount 0.5, slewCoeff 0.05, limit 0.6.
        const float clipAmount = 0.3f;
        const float slewCoeff = 0.06f;
        const float smoothedPrimary = shapeAndSlew(output * 1.5f, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(secondary * 1.5f, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 1.2f);
    }

private:
    float sampleRate;
    float phase;
    float modPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
