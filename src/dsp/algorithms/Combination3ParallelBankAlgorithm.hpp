#pragma once

#include "AlgorithmOutput.hpp"
#include "AlgorithmUtils.hpp"

namespace flues::disyn {

class Combination3ParallelBankAlgorithm {
public:
    explicit Combination3ParallelBankAlgorithm(float sampleRate)
        : sampleRate(sampleRate),
          parallel1Phase(0.0f),
          parallel2Phase(0.0f),
          parallel3Phase(0.0f),
          parallel4Phase(0.0f),
          parallel5Phase(0.0f),
          formant1Phase(0.0f),
          formant2Phase(0.0f),
          formant3Phase(0.0f),
          outPrimary(0.0f),
          outSecondary(0.0f) {}

    void reset() {
        parallel1Phase = 0.0f;
        parallel2Phase = 0.0f;
        parallel3Phase = 0.0f;
        parallel4Phase = 0.0f;
        parallel5Phase = 0.0f;
        formant1Phase = 0.0f;
        formant2Phase = 0.0f;
        formant3Phase = 0.0f;
        outPrimary = 0.0f;
        outSecondary = 0.0f;
    }

    AlgorithmOutput process(float pitch, float param1, float param2, float param3) {
        (void)param2;
        (void)param1;
        const float mixBalance = std::clamp(param3, 0.0f, 1.0f);

        // Prev tune: simplified sines + formants, raw *0.3, clipAmount 0.7, slewCoeff 0.04, limit 0.4.
        parallel1Phase = stepPhase(parallel1Phase, pitch, sampleRate);
        parallel3Phase = stepPhase(parallel3Phase, pitch * 1.5f, sampleRate);
        parallel5Phase = stepPhase(parallel5Phase, pitch * 2.0f, sampleRate);

        const float voice1 = std::sin(TWO_PI * parallel1Phase) * 0.5f;
        const float voice2 = std::sin(TWO_PI * parallel3Phase) * 0.5f;
        const float voice3 = std::sin(TWO_PI * parallel5Phase) * 0.5f;

        formant2Phase = stepPhase(formant2Phase, 800.0f, sampleRate);
        formant3Phase = stepPhase(formant3Phase, 2400.0f, sampleRate);
        const float paf1 = std::sin(TWO_PI * formant2Phase) * 0.4f;
        const float paf2 = std::sin(TWO_PI * formant3Phase) * 0.4f;

        const float voiceMix = (voice1 + voice2 + voice3) / 3.0f;
        const float pafMix = (paf1 + paf2) / 2.0f;
        const float rawPrimary = (voiceMix * (1.0f - mixBalance) + pafMix * mixBalance) * 1.0f;
        const float rawSecondary = voiceMix * 1.0f;
        const float clipAmount = 0.3f;
        const float slewCoeff = 0.06f;
        const float smoothedPrimary = shapeAndSlew(rawPrimary, outPrimary, slewCoeff, clipAmount);
        const float smoothedSecondary = shapeAndSlew(rawSecondary, outSecondary, slewCoeff, clipAmount);
        return normalizeOutputLimit(smoothedPrimary, smoothedSecondary, 1.0f);
    }

private:
    float sampleRate;
    float parallel1Phase;
    float parallel2Phase;
    float parallel3Phase;
    float parallel4Phase;
    float parallel5Phase;
    float formant1Phase;
    float formant2Phase;
    float formant3Phase;
    float outPrimary;
    float outSecondary;
};

} // namespace flues::disyn
