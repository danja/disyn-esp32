# Danger Zones (ESP32)

This project has a few code paths that have caused full system failure
(blank display, no serial output) when deployed to ESP32 hardware.
Keep these in mind when modifying DSP or utilities.

## Known failure triggers
- Adding `engine.setParam3()` in `src/dsp/DspTask.cpp` (even with a constant
  0.5) causes a boot-time crash / blank display.
- Refactoring `src/dsp/modules/OscillatorModule.hpp` to store the switch
  output in a local variable and return later (instead of returning directly
  from each case) causes a crash. This was introduced to apply a
  NaN/Inf sanitizer at the end of the function.
- NaN/Inf guards that use `std::numeric_limits`, `std::isfinite`, or manual
  IEEE-754 checks in the oscillator output path appear to trigger the same
  failure, likely due to the switch refactor or the guard itself.

## Notes
- Plain UI-only changes (menu layout, label tweaks) and encoder debounce are
  stable.
- Gate output inversion in `src/dsp/DspTask.cpp` is stable.
- Label-only changes in `include/AlgorithmInfo.h` are stable.

## Next investigations
- Find the minimal change inside `OscillatorModule::process()` that triggers
  the crash (local variable? stack usage? sanitizer codegen?).
- Isolate `engine.setParam3()` in `DisynEngine` to see if the crash is in the
  setter or elsewhere in the DSP path.
