# Wavefolder Plan + Findings (ESP32)

Goal: Add a wavefolder stage immediately after the oscillator, controlled by
knob/CV 1 (pot0/cv0), while keeping knob/CV 3 (pot2/cv2) on pitch.

## Changes attempted
- Added `WavefolderModule` and inserted it after the oscillator in
  `DisynEngine::process()`.
- Routed wavefold amount from pot0/cv0 in `DspTask.cpp`.
- Moved wavefolder out of the engine into `DspTask.cpp` (post-engine fold).
- Tried minimal fold math (no `fmod`/`fabs`) to rule out heavy math.
- Tried static local wavefolder vs member storage.
- Tried removing the wavefold amount setter call.

## Results
All variants above crashed on hardware (blank screen / no boot), even when the
wavefolder was a no-op or used only in `DspTask.cpp`.

## Current state
All wavefolder changes reverted; repo restored to last known-good boot state.

## Next steps to try
1) Confirm stability of the clean build/flash (no wavefolder references).
2) If stable, reintroduce the wavefolder incrementally but outside the DSP task
   (e.g., in a separate test build or using a dummy inline operation with no
   new includes) to isolate the trigger.
3) If any wavefolder usage causes crashes, investigate ESP32 toolchain/ABI
   issues or memory constraints in DSP code paths.
