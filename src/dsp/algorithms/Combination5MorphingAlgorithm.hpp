#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination5MorphingAlgorithm {
public:
    explicit Combination5MorphingAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          phase(0.0f),
          modPhase(0.0f),
          secondaryPhase(0.0f),
          formant1Phase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        phase = 0.0f;
        modPhase = 0.0f;
        secondaryPhase = 0.0f;
        formant1Phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        const float morphCurve = 0.5f + std::clamp(param3, 0.0f, 1.0f) * 1.5f;
        const float morphPos = std::pow(std::clamp(param1, 0.0f, 1.0f), morphCurve);
        (void)param2;

        float output = 0.0f;

        float secondary = 0.0f;

        if (morphPos < 0.5f) {
            const float alpha = morphPos * 2.0f;

            phase = stepPhase(phase, pitch, sampleRate);
            const float sine = std::sin(TWO_PI * phase) * 0.5f;

            modPhase = stepPhase(modPhase, pitch, sampleRate);
            const float modfm = std::sin(TWO_PI * modPhase) * 0.5f;

            output = sine * (1.0f - alpha) + modfm * alpha;
            secondary = modfm;
        } else {
            const float alpha = (morphPos - 0.5f) * 2.0f;

            modPhase = stepPhase(modPhase, pitch, sampleRate);
            const float modfm = std::sin(TWO_PI * modPhase) * 0.5f;

            formant1Phase = stepPhase(formant1Phase, pitch * 2.0f, sampleRate);
            const float paf = std::sin(TWO_PI * formant1Phase) * 0.5f;

            output = modfm * (1.0f - alpha) + paf * alpha;
            secondary = paf;
        }

        // Prev tune: raw *0.9, clipAmount 0.3, slewCoeff 0.06, limit 1.0.
        const float rawPrimary = output * 0.8f;
        const float rawSecondary = secondary * 0.8f;
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
    float formant1Phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
