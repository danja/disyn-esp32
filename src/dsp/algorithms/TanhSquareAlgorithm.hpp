#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class TanhSquareAlgorithm {
public:
    explicit TanhSquareAlgorithm(float sampleRate)
        : sampleRate(sampleRate), phase(0.0f), outPrimary(0.0f), outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        const float drive = expoMap(param1, 0.05f, 5.0f);
        const float trim = expoMap(param2, 0.2f, 1.2f);
        const float bias = (std::clamp(param3, 0.0f, 1.0f) - 0.5f) * 0.8f;

        phase = stepPhase(phase, pitch, sampleRate);
        const float carrier = std::sin(phase * TWO_PI) + bias;
        const float rawPrimary = clampAbs(std::tanh(carrier * drive) * trim, 1.0f);
        const float rawSecondary = clampAbs(std::tanh(std::sin(phase * TWO_PI) * drive) * trim, 1.0f);
        const float clipAmount = 0.0f;
        const float slewCoeff = 0.2f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutput(smoothedPrimary, smoothedSecondary);
    }

private:
    float sampleRate;
    float phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
