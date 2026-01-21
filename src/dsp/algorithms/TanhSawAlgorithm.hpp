#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class TanhSawAlgorithm {
public:
    explicit TanhSawAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          secondaryPhase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        secondaryPhase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        const float drive = expoMap(param1, 0.05f, 4.5f);
        const float blend = std::clamp(param2, 0.0f, 1.0f);
        const float edge = 0.5f + std::clamp(param3, 0.0f, 1.0f) * 1.5f;

        phase = stepPhase(phase, pitch, sampleRate);
        const float sine = std::sin(phase * TWO_PI);
        const float square = std::tanh(sine * drive);

        secondaryPhase = stepPhase(secondaryPhase, pitch, sampleRate);
        const float cosine = std::cos(secondaryPhase * TWO_PI);
        const float saw = square + cosine * (1.0f - square * square) * edge;

        const float raw = square * (1.0f - blend) + saw * blend;
        const float rawPrimary = std::tanh(raw);
        const float rawSecondary = square;
        const float clipAmount = 0.0f;
        const float slewCoeff = 0.2f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutput(smoothedPrimary, smoothedSecondary);
    }

private:
    float sampleRate;
    float phase;
    float secondaryPhase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
