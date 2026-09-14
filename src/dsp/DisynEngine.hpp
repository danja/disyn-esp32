#pragma once

#include <cmath>
#include <algorithm>

#include "algorithms/AlgorithmOutput.hpp"
#include "algorithms/AlgorithmTypes.hpp"
#include "modules/OscillatorModule.hpp"
#include "modules/WavefolderModule.hpp"
#include "modules/EnvelopeModule.hpp"
#include "modules/ReverbModule.hpp"

namespace flues::disyn
{

    class DisynEngine
    {
    public:
        explicit DisynEngine(float sampleRate = 44100.0f)
            : sampleRate(sampleRate),
              oscillator(sampleRate),
              wavefolder(),
              envelope(sampleRate),
              reverbLeft(sampleRate),
              reverbRight(sampleRate),
              frequency(440.0f),
              algorithmType(AlgorithmType::TANH_SQUARE),
              param1(0.55f), // Default drive for tanh square
              param2(0.5f),  // Default trim for tanh square
              param3(0.5f),
              wavefoldAmount(0.0f),
              masterGain(0.8f),
              velocity(1.0f),
              gate(false),
              isPlaying(false)
        {
        }

        void noteOn(float freq, float vel = 1.0f)
        {
            frequency = freq;
            velocity = std::clamp(vel, 0.0f, 1.0f);
            gate = true;
            isPlaying = true;

            oscillator.reset();
            envelope.reset();
            reverbLeft.reset();
            reverbRight.reset();

            envelope.setGate(true);
        }

        void noteOff()
        {
            gate = false;
            envelope.setGate(false);
        }

        void setFrequency(float freq)
        {
            frequency = std::max(freq, 0.0f);
        }

        AlgorithmOutput process()
        {
            if (!isPlaying)
            {
                return {0.0f, 0.0f};
            }

            // Generate oscillator sample
            const AlgorithmOutput oscOutput = oscillator.process(algorithmType, frequency, param1, param2, param3);
            float primary = oscOutput.primary;
            float secondary = oscOutput.secondary;
            const float foldGain = getAlgorithmFoldGain(algorithmType);
            const float outputGain = getAlgorithmOutputGain(algorithmType);
            const float preFoldPrimary = primary * foldGain;
            const float preFoldSecondary = secondary * foldGain;
            const float foldedPrimary = wavefolder.process(preFoldPrimary, wavefoldAmount);
            const float foldedSecondary = wavefolder.process(preFoldSecondary, wavefoldAmount);
            // Prev tune: postGain 3.0 with tanh. Reverting to avoid global distortion.
            const float postPrimary = foldedPrimary;
            const float postSecondary = foldedSecondary;

            // Apply envelope
            const float env = envelope.process();

            // Apply velocity and master gain
            const float leftSample = postPrimary * env * velocity * masterGain * outputGain;
            const float rightSample = postSecondary * env * velocity * masterGain * outputGain;

            // Apply reverb
            const float left = reverbLeft.process(leftSample);
            const float right = reverbRight.process(rightSample);

            // Voice tail detection - stop if envelope is silent
            if (!envelope.isPlaying() &&
                std::max(std::abs(left), std::abs(right)) < 1e-5f)
            {
                isPlaying = false;
            }

            return {left, right};
        }

        // Parameter setters
        void setAlgorithm(int type)
        {
            if (type >= 0 && type <= 25)
            {
                algorithmType = static_cast<AlgorithmType>(type);
            }
        }

        void setParam1(float value)
        {
            param1 = std::clamp(value, 0.0f, 1.0f);
        }

        void setParam2(float value)
        {
            param2 = std::clamp(value, 0.0f, 1.0f);
        }

        void setParam3(float value)
        {
            param3 = std::clamp(value, 0.0f, 1.0f);
        }

        void setWavefoldAmount(float value)
        {
            wavefoldAmount = std::clamp(value, 0.0f, 1.0f);
        }

        void setAttack(float value)
        {
            envelope.setAttack(value);
        }

        void setRelease(float value)
        {
            envelope.setRelease(value);
        }

        void setReverbSize(float value)
        {
            reverbLeft.setSize(value);
            reverbRight.setSize(value);
        }

        void setReverbLevel(float value)
        {
            reverbLeft.setLevel(value);
            reverbRight.setLevel(value);
        }

        void setMasterGain(float value)
        {
            masterGain = std::clamp(value, 0.0f, 1.0f);
        }

        bool getIsPlaying() const
        {
            return isPlaying;
        }

    private:
        // Normalizes each algorithm's worst-case peak to ~1.0 going INTO the
        // wavefolder. Measured on host over a pitch x param1 x param2 x param3
        // sweep; see tools/audit_algorithms.cpp. Note this gain is applied even
        // when wavefoldAmount is 0 (the folder is a passthrough there), so
        // getAlgorithmOutputGain below has to undo it -- the two are a pair.
        static float getAlgorithmFoldGain(AlgorithmType type)
        {
            switch (type)
            {
            case AlgorithmType::DIRICHLET_PULSE:
                return 2.500000f;
            case AlgorithmType::DSF_SINGLE:
                return 2.597963f;
            case AlgorithmType::DSF_DOUBLE:
                return 2.595751f;
            case AlgorithmType::TANH_SQUARE:
                return 1.666667f;
            case AlgorithmType::TANH_SAW:
                return 1.666667f;
            case AlgorithmType::PAF:
                return 2.100840f;
            case AlgorithmType::MOD_FM:
                return 2.099685f;
            case AlgorithmType::COMBINATION_1_HYBRID_FORMANT:
                return 2.127320f;
            case AlgorithmType::COMBINATION_2_CASCADED:
                return 1.218119f;
            case AlgorithmType::COMBINATION_3_PARALLEL_BANK:
                return 2.265195f;
            case AlgorithmType::COMBINATION_4_FEEDBACK:
                return 1.777497f;
            case AlgorithmType::COMBINATION_5_MORPHING:
                return 2.570806f;
            case AlgorithmType::COMBINATION_6_INHARMONIC:
                return 2.500000f;
            case AlgorithmType::COMBINATION_7_ADAPTIVE_FILTER:
                return 1.341177f;
            case AlgorithmType::NOVEL_1_MULTISTAGE:
                return 1.000000f;
            case AlgorithmType::NOVEL_2_FREQ_ASYMMETRY:
                return 1.725664f;
            case AlgorithmType::NOVEL_3_CROSS_MOD:
                return 0.833333f;
            case AlgorithmType::NOVEL_4_TAYLOR:
                return 1.666667f;
            case AlgorithmType::TRAJECTORY:
                return 2.000000f;
            case AlgorithmType::SINE:
                return 1.000000f;
            case AlgorithmType::RAMP:
                return 1.000000f;
            case AlgorithmType::TRIANGLE:
                return 1.000000f;
            case AlgorithmType::PULSE:
                return 1.000000f;
            case AlgorithmType::NOISE:
                return 1.000172f;
            case AlgorithmType::LOGISTIC:
                return 1.053167f;
            case AlgorithmType::BUTTERFLY:
                return 1.132249f;
            default:
                return 1.0f;
            }
        }

        // Restores level after getAlgorithmFoldGain and equalizes loudness across
        // algorithms: chosen so measured RMS lands at ~0.28 of full scale after
        // masterGain and kGlobalPreGain. Capped at 1.0, so a few high-crest
        // algorithms sit slightly below target rather than clipping.
        static float getAlgorithmOutputGain(AlgorithmType type)
        {
            switch (type)
            {
            case AlgorithmType::DIRICHLET_PULSE:
                return 0.508842f;
            case AlgorithmType::DSF_SINGLE:
                return 0.703299f;
            case AlgorithmType::DSF_DOUBLE:
                return 1.000000f;
            case AlgorithmType::TANH_SQUARE:
                return 0.547860f;
            case AlgorithmType::TANH_SAW:
                return 0.580948f;
            case AlgorithmType::PAF:
                return 0.770953f;
            case AlgorithmType::MOD_FM:
                return 0.704991f;
            case AlgorithmType::COMBINATION_1_HYBRID_FORMANT:
                return 1.000000f;
            case AlgorithmType::COMBINATION_2_CASCADED:
                return 0.573782f;
            case AlgorithmType::COMBINATION_3_PARALLEL_BANK:
                return 1.000000f;
            case AlgorithmType::COMBINATION_4_FEEDBACK:
                return 0.579525f;
            case AlgorithmType::COMBINATION_5_MORPHING:
                return 0.706086f;
            case AlgorithmType::COMBINATION_6_INHARMONIC:
                return 0.644000f;
            case AlgorithmType::COMBINATION_7_ADAPTIVE_FILTER:
                return 0.533290f;
            case AlgorithmType::NOVEL_1_MULTISTAGE:
                return 0.669072f;
            case AlgorithmType::NOVEL_2_FREQ_ASYMMETRY:
                return 0.682935f;
            case AlgorithmType::NOVEL_3_CROSS_MOD:
                return 0.676186f;
            case AlgorithmType::NOVEL_4_TAYLOR:
                return 0.555449f;
            case AlgorithmType::TRAJECTORY:
                return 0.628248f;
            case AlgorithmType::SINE:
                return 0.702198f;
            case AlgorithmType::RAMP:
                return 0.854677f;
            case AlgorithmType::TRIANGLE:
                return 0.861335f;
            case AlgorithmType::PULSE:
                return 0.500000f;
            case AlgorithmType::NOISE:
                return 0.834628f;
            case AlgorithmType::LOGISTIC:
                return 0.776272f;
            case AlgorithmType::BUTTERFLY:
                return 1.000000f;
            default:
                return 1.0f;
            }
        }
        float sampleRate;
        OscillatorModule oscillator;
        WavefolderModule wavefolder;
        EnvelopeModule envelope;
        ReverbModule reverbLeft;
        ReverbModule reverbRight;

        float frequency;
        AlgorithmType algorithmType;
        float param1;
        float param2;
        float param3;
        float wavefoldAmount;
        float masterGain;
        float velocity;
        bool gate;
        bool isPlaying;
    };

} // namespace flues::disyn
