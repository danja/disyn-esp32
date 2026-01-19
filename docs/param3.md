# Param3 Plan (Deferred)

Goal: Use param3 as an additional per-algorithm control so all three knobs/CVs
affect every algorithm without breaking ESP32 stability.

Findings so far:
- Adding `engine.setParam3()` in `src/dsp/DspTask.cpp` causes a blank screen /
  system crash on ESP32, even with a constant 0.5 value.
- The failure appears in the DSP task path, not the UI.

Next steps (when resuming):
1) Audit `flues::disyn::DisynEngine::setParam3()` and any internal uses in the
   DSP path for unsafe operations on ESP32.
2) Add a minimal, isolated test: set a constant param3 inside the engine class
   (not from `DspTask`) to confirm whether the crash is in `setParam3()` or
   elsewhere in `DspTask`.
3) If safe, reintroduce param3 in `DspTask` with hard-coded 0.5 and no algorithm
   switches; verify stability.
4) If stable, map param3 to existing algorithms that already support it:
   - C1 Hybrid Formant -> use param2 for formant spacing
   - C3 Parallel Bank -> use param2 for mix balance
5) Add display labels for param2 where it now controls the param3 behavior:
   "Space" for Formant, "Mix" for Banks.
6) Only after stability: consider exposing param3 as a third UI parameter or
   mapping via CV2/Pot2.
