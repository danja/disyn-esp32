// Host-side audit of every oscillator algorithm, driven directly so the
// isAlgorithmActive() mute gate in OscillatorModule does not zero the output.
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "dsp/algorithms/ButterflyAlgorithm.hpp"
#include "dsp/algorithms/Combination1HybridFormantAlgorithm.hpp"
#include "dsp/algorithms/Combination2CascadedAlgorithm.hpp"
#include "dsp/algorithms/Combination3ParallelBankAlgorithm.hpp"
#include "dsp/algorithms/Combination4FeedbackAlgorithm.hpp"
#include "dsp/algorithms/Combination5MorphingAlgorithm.hpp"
#include "dsp/algorithms/Combination6InharmonicAlgorithm.hpp"
#include "dsp/algorithms/Combination7AdaptiveFilterAlgorithm.hpp"
#include "dsp/algorithms/DirichletPulseAlgorithm.hpp"
#include "dsp/algorithms/DSFDoubleAlgorithm.hpp"
#include "dsp/algorithms/DSFSingleAlgorithm.hpp"
#include "dsp/algorithms/LogisticAlgorithm.hpp"
#include "dsp/algorithms/ModFMAlgorithm.hpp"
#include "dsp/algorithms/NoiseAlgorithm.hpp"
#include "dsp/algorithms/Novel1MultistageAlgorithm.hpp"
#include "dsp/algorithms/Novel2FreqAsymmetryAlgorithm.hpp"
#include "dsp/algorithms/Novel3CrossModAlgorithm.hpp"
#include "dsp/algorithms/Novel4TaylorAlgorithm.hpp"
#include "dsp/algorithms/PAFAlgorithm.hpp"
#include "dsp/algorithms/PulseAlgorithm.hpp"
#include "dsp/algorithms/RampAlgorithm.hpp"
#include "dsp/algorithms/SineAlgorithm.hpp"
#include "dsp/algorithms/TanhSawAlgorithm.hpp"
#include "dsp/algorithms/TanhSquareAlgorithm.hpp"
#include "dsp/algorithms/TrajectoryAlgorithm.hpp"
#include "dsp/algorithms/TriangleAlgorithm.hpp"

using namespace flues::disyn;

static const float kSR = 44100.0f;

struct Result {
    double peak = 0.0;      // worst-case peak across the sweep
    double rmsMax = 0.0;    // loudest RMS across the sweep
    double rmsMin = 1e9;    // quietest RMS across the sweep
    double dcWorst = 0.0;   // worst |mean|
    double growth = 1.0;    // RMS(last quarter) / RMS(first quarter), worst case
    int nonFinite = 0;      // NaN/Inf samples
    int silentCells = 0;    // sweep cells with RMS < 1e-4
    int totalCells = 0;
};

template <typename Algo>
Result audit() {
    const std::array<float, 5> grid = {0.0f, 0.25f, 0.5f, 0.75f, 1.0f};
    const std::array<float, 3> pitches = {55.0f, 220.0f, 1760.0f};
    const int N = 8192;
    Result r;

    for (float pitch : pitches) {
        for (float p1 : grid) {
            for (float p2 : grid) {
                for (float p3 : grid) {
                    Algo algo(kSR);
                    algo.reset();
                    double sumSq = 0.0, sum = 0.0;
                    double sqFirst = 0.0, sqLast = 0.0;
                    double peak = 0.0;
                    int nf = 0;
                    for (int i = 0; i < N; ++i) {
                        const AlgorithmOutput o = algo.process(pitch, p1, p2, p3);
                        const double v = o.primary;
                        if (!std::isfinite(v) || !std::isfinite(o.secondary)) { ++nf; continue; }
                        sum += v;
                        sumSq += v * v;
                        peak = std::max(peak, std::max(std::abs(v), std::abs((double)o.secondary)));
                        if (i < N / 4) sqFirst += v * v;
                        if (i >= 3 * N / 4) sqLast += v * v;
                    }
                    const double rms = std::sqrt(sumSq / N);
                    const double a = std::sqrt(sqFirst / (N / 4));
                    const double b = std::sqrt(sqLast / (N / 4));
                    r.peak = std::max(r.peak, peak);
                    r.rmsMax = std::max(r.rmsMax, rms);
                    r.rmsMin = std::min(r.rmsMin, rms);
                    r.dcWorst = std::max(r.dcWorst, std::abs(sum / N));
                    if (a > 1e-6) r.growth = std::max(r.growth, b / a);
                    r.nonFinite += nf;
                    if (rms < 1e-4) ++r.silentCells;
                    ++r.totalCells;
                }
            }
        }
    }
    return r;
}

struct Row { const char* name; const char* bads; Result r; const char* en; };

int main() {
    std::vector<Row> rows;
    auto add = [&](const char* n, const char* b, Result r, const char* e){ rows.push_back({n,b,r,e}); };

    add("Dirichlet",  "g", audit<DirichletPulseAlgorithm>(), "DIRICHLET_PULSE");
    add("DSF Single", "b", audit<DSFSingleAlgorithm>(), "DSF_SINGLE");
    add("DSF Double", "b", audit<DSFDoubleAlgorithm>(), "DSF_DOUBLE");
    add("Tanh Square","b", audit<TanhSquareAlgorithm>(), "TANH_SQUARE");
    add("Tanh Saw",   "b", audit<TanhSawAlgorithm>(), "TANH_SAW");
    add("PAF",        "i", audit<PAFAlgorithm>(), "PAF");
    add("Mod FM",     "b", audit<ModFMAlgorithm>(), "MOD_FM");
    add("Formant",    "g", audit<Combination1HybridFormantAlgorithm>(), "COMBINATION_1_HYBRID_FORMANT");
    add("Cascade",    "s", audit<Combination2CascadedAlgorithm>(), "COMBINATION_2_CASCADED");
    add("Banks",      "s", audit<Combination3ParallelBankAlgorithm>(), "COMBINATION_3_PARALLEL_BANK");
    add("Feedback",   "g", audit<Combination4FeedbackAlgorithm>(), "COMBINATION_4_FEEDBACK");
    add("Morphing",   "i", audit<Combination5MorphingAlgorithm>(), "COMBINATION_5_MORPHING");
    add("Inharmonic", "b", audit<Combination6InharmonicAlgorithm>(), "COMBINATION_6_INHARMONIC");
    add("AFilter",    "s", audit<Combination7AdaptiveFilterAlgorithm>(), "COMBINATION_7_ADAPTIVE_FILTER");
    add("Multi",      "i", audit<Novel1MultistageAlgorithm>(), "NOVEL_1_MULTISTAGE");
    add("Asym",       "s", audit<Novel2FreqAsymmetryAlgorithm>(), "NOVEL_2_FREQ_ASYMMETRY");
    add("Cross",      "s", audit<Novel3CrossModAlgorithm>(), "NOVEL_3_CROSS_MOD");
    add("Taylor",     "g", audit<Novel4TaylorAlgorithm>(), "NOVEL_4_TAYLOR");
    add("Trajectory", "g", audit<TrajectoryAlgorithm>(), "TRAJECTORY");
    add("Sine",       "-", audit<SineAlgorithm>(), "SINE");
    add("Ramp",       "-", audit<RampAlgorithm>(), "RAMP");
    add("Triangle",   "-", audit<TriangleAlgorithm>(), "TRIANGLE");
    add("Pulse",      "-", audit<PulseAlgorithm>(), "PULSE");
    add("Noise",      "-", audit<NoiseAlgorithm>(), "NOISE");
    add("Logistic",   "-", audit<LogisticAlgorithm>(), "LOGISTIC");
    add("Butterfly",  "-", audit<ButterflyAlgorithm>(), "BUTTERFLY");

    printf("%-12s %4s %10s %9s %9s %8s %8s %7s %9s\n",
           "Algorithm","bads","peak","rmsMax","rmsMin","DC","growth","NaN","silent");
    printf("%s\n", std::string(88,'-').c_str());
    for (auto &x : rows) {
        printf("%-12s %4s %10.3f %9.4f %9.4f %8.3f %8.2f %7d %5d/%d\n",
               x.name, x.bads, x.r.peak, x.r.rmsMax, x.r.rmsMin,
               x.r.dcWorst, x.r.growth, x.r.nonFinite,
               x.r.silentCells, x.r.totalCells);
    }
    printf("\n#CSV\n");
    for (auto &x : rows) printf("%s,%.6f,%.6f\n", x.en, x.r.peak, x.r.rmsMax);
    return 0;
}
