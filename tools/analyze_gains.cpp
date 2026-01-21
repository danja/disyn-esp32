#include <algorithm>
#include <array>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>

#include "dsp/modules/OscillatorModule.hpp"
#include "dsp/algorithms/AlgorithmTypes.hpp"

namespace {
struct Stats {
    double peak = 0.0;
    double rms = 0.0;
};

Stats analyzeAlgorithm(flues::disyn::AlgorithmType type, float pitch,
                      const std::array<float, 5> &grid, int samples) {
    flues::disyn::OscillatorModule osc(44100.0f);
    double worstPeak = 0.0;
    double worstRms = 0.0;

    for (float p1 : grid) {
        for (float p2 : grid) {
            for (float p3 : grid) {
                osc.reset();
                double sumSq = 0.0;
                double peak = 0.0;
                for (int i = 0; i < samples; ++i) {
                    const auto out = osc.process(type, pitch, p1, p2, p3);
                    const double v = std::max(std::abs(out.primary), std::abs(out.secondary));
                    peak = std::max(peak, v);
                    sumSq += static_cast<double>(out.primary) * static_cast<double>(out.primary);
                }
                const double rms = std::sqrt(sumSq / static_cast<double>(samples));
                worstPeak = std::max(worstPeak, peak);
                worstRms = std::max(worstRms, rms);
            }
        }
    }

    return {worstPeak, worstRms};
}
}

int main() {
    const std::array<float, 5> grid = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    const float pitch = 220.0f;
    const int samples = 4096;
    const double targetPeak = 1.2;
    const double targetRms = 0.4;

    struct AlgoEntry {
        flues::disyn::AlgorithmType type;
        const char *name;
    };

    const std::array<AlgoEntry, 19> algos = {{
        {flues::disyn::AlgorithmType::DIRICHLET_PULSE, "Dirichlet"},
        {flues::disyn::AlgorithmType::DSF_SINGLE, "DSF Single"},
        {flues::disyn::AlgorithmType::DSF_DOUBLE, "DSF Double"},
        {flues::disyn::AlgorithmType::TANH_SQUARE, "Tanh Square"},
        {flues::disyn::AlgorithmType::TANH_SAW, "Tanh Saw"},
        {flues::disyn::AlgorithmType::PAF, "PAF"},
        {flues::disyn::AlgorithmType::MOD_FM, "Mod FM"},
        {flues::disyn::AlgorithmType::COMBINATION_1_HYBRID_FORMANT, "Formant"},
        {flues::disyn::AlgorithmType::COMBINATION_2_CASCADED, "Cascade"},
        {flues::disyn::AlgorithmType::COMBINATION_3_PARALLEL_BANK, "Banks"},
        {flues::disyn::AlgorithmType::COMBINATION_4_FEEDBACK, "Feedback"},
        {flues::disyn::AlgorithmType::COMBINATION_5_MORPHING, "Morphing"},
        {flues::disyn::AlgorithmType::COMBINATION_6_INHARMONIC, "Inharmonic"},
        {flues::disyn::AlgorithmType::COMBINATION_7_ADAPTIVE_FILTER, "AFilter"},
        {flues::disyn::AlgorithmType::NOVEL_1_MULTISTAGE, "Multi"},
        {flues::disyn::AlgorithmType::NOVEL_2_FREQ_ASYMMETRY, "Asym"},
        {flues::disyn::AlgorithmType::NOVEL_3_CROSS_MOD, "Cross"},
        {flues::disyn::AlgorithmType::NOVEL_4_TAYLOR, "Taylor"},
        {flues::disyn::AlgorithmType::TRAJECTORY, "Trajectory"},
    }};

    std::cout << "algorithm,peak,rms,fold_gain,out_gain" << std::endl;
    for (const auto &algo : algos) {
        const Stats stats = analyzeAlgorithm(algo.type, pitch, grid, samples);
        const double foldGain = stats.peak > 0.0 ? std::min(1.0, targetPeak / stats.peak) : 1.0;
        const double outGain = stats.rms > 0.0 ? std::min(2.0, targetRms / (stats.rms * foldGain)) : 1.0;
        std::cout << algo.name << ","
                  << std::fixed << std::setprecision(4)
                  << stats.peak << "," << stats.rms << "," << foldGain << "," << outGain << std::endl;
    }

    return 0;
}
