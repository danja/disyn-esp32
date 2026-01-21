# Safer output notes

This document summarizes likely sources of loud or unstable output in the current
oscillator algorithms and why gain scaling alone may not prevent spikes.

## Summary of likely hot spots

- DirichletPulse: Near poles at sin(theta/2) ~= 0 can create sharp spikes even with
  the EPSILON guard. The +18 dB tilt option can further increase peaks.
- DSF Single/Double: As decay approaches 1.0, the denominator in the DSF formula
  can get very small and create large peaks. EPSILON prevents division by zero but
  does not cap the peak magnitude.
- ModFM and ModFM-derived combinations: The exp term can expand peaks when the
  modulator is near -1.0. The exp(-index) envelope only fully cancels at mod=1.
- Asymmetric FM (processAsymmetricFM): With k up to 10 and r up to 2, the exp term
  can reach ~exp(7.5) (~1800). Algorithms that use this directly can spike.
- Feedback algorithm: Frequency feedback combined with exp modulation can chirp
  or blow up when feedback approaches 1.
- Combination algorithms: These inherit peak risk from DSF/ModFM/Asymmetric FM
  and can be louder because of mixing and partial summing.
- Novel1Multistage: expDepth * stage1 can produce gains up to ~exp(1.5) before the
  ring modulation stage.
- Novel4Taylor and Trajectory: Already bounded and generally stable.

## Implication

Per-algorithm safety measures (peak limiting, tighter parameter ranges, or output
normalization) are needed. The static gain table in DisynEngine reduces average
levels but cannot fully prevent short-duration spikes or feedback excursions.
