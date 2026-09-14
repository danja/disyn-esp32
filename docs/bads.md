# Algorithm Audit

Previous verdicts (by ear, on hardware):
g = good, b = bad/distorted instability, i = intermittent, s = silent

Those verdicts drove a mute list in `OscillatorModule::isAlgorithmActive()` that
silenced 15 of 26 algorithms. A host-side measurement pass has since shown the
verdicts were mostly measuring a gain bug, not the algorithms. The mute list has
been removed.

## What the gain bug was

`DisynEngine::process()` scales the oscillator by `getAlgorithmFoldGain()` before
the wavefolder. That table held values as low as 0.0004, and the wavefolder is a
plain passthrough when `wavefoldAmount` is 0, so nothing ever restored the level.
`getAlgorithmOutputGain()` (0.2-1.0) came nowhere near compensating.

Effective gain spread across algorithms was about **5700:1**. On the 8-bit DAC:

| Algorithm | old foldGain | peak output |
|---|---|---|
| AFilter | 0.0004 | 0.02 of 255 codes |
| Asym | 0.000699 | 0.02 codes |
| Cross | 0.01 | 1.3 codes |
| Cascade | 0.01 | 0.4 codes |
| Pulse | 1.0 | 114 codes |

Every algorithm marked `s` (silent) was simply attenuated below one DAC code.
Every `i` (intermittent) was at 0.15-0.35 codes. Both tables are now derived from
measurement and all algorithms land within 2:1 of each other (71-143 codes peak).

## Running the audit

```bash
g++ -std=gnu++17 -O2 -I src -I include -o /tmp/audit tools/audit_algorithms.cpp && /tmp/audit
```

Drives each algorithm directly (bypassing the active-set gate) over a sweep of
3 pitches x 5^3 param values x 8192 samples, reporting peak, RMS range, DC
offset, amplitude growth, non-finite sample count and silent-cell count. The
`#CSV` block at the end feeds the gain tables in `DisynEngine.hpp`.

## Bugs found and fixed

- **Butterfly produced NaN/Inf.** It integrates a Lorenz attractor with forward
  Euler, and `dt` was clamped to 0.05. Euler on Lorenz diverges above dt ~= 0.023,
  so every pitch above ~970 Hz blew the state to infinity within ~25 samples —
  1,019,875 non-finite samples across the sweep. Now sub-steps at dt <= 0.02 with
  a bounded-range guard. Zero non-finite samples.
- **Master gain was applied twice**, in `DisynEngine::process()` and again in
  `DspTask`, making everything ~44% quieter than intended.
- **Feedback was muted despite being marked `g`.** Oversight.

## Still worth watching

- **Pulse** has DC offset up to 0.902 and RMS 1.0 at every setting — at extreme
  widths it is close to a constant level, not a waveform. Through a DC-coupled
  output that is a thump. Candidate for DC blocking.
- **DSF Double** is genuinely silent in 75 of 375 sweep cells (20% of its
  parameter space), independent of gain.
- **Trajectory** reads as silent in 4 cells, but only on the primary channel:
  launching at 90/270 degrees into an even-sided polygon with bounce jitter at 0
  gives a degenerate vertical orbit where `position.x` stays at zero. The
  secondary channel is fine and any jitter breaks the degeneracy. Working as
  designed.
- All the old `b` (distorted) verdicts are untested since the recalibration. They
  were being driven at wildly wrong levels, so those judgements need redoing by
  ear now that levels are even.
