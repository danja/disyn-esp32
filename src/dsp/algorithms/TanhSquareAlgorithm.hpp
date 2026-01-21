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
        // Prev tune: drive max 5.0, trim max 1.2, bias range +/-0.4, raw *1.2.
        const float drive = expoMap(param1, 0.05f, 2.5f);
        const float trim = expoMap(param2, 0.3f, 0.9f);
        const float bias = (std::clamp(param3, 0.0f, 1.0f) - 0.5f) * 0.3f;

        phase = stepPhase(phase, pitch, sampleRate);
        const float carrier = std::sin(phase * TWO_PI) + bias;
        // Prev tune: raw *1.2, clipAmount 0.4, slewCoeff 0.1, limit 0.8.
        const float rawPrimary = clampAbs(std::tanh(carrier * drive) * trim, 1.0f) * 0.9f;
        const float rawSecondary = clampAbs(std::tanh(std::sin(phase * TWO_PI) * drive) * trim, 1.0f) * 0.9f;
        const float clipAmount = 0.6f;
        const float slewCoeff = 0.08f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 0.6f);
    }

private:
    float sampleRate;
    float phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
