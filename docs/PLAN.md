# Implementation Plan

## Goals
- Build ESP32-based Eurorack module firmware with dual-core split: UI on one core, DSP on the other.
- Support the listed hardware IO (OLED, encoder, gate IO, CV/pot ADCs, DAC outputs).
- Implement DSP algorithms based on `reference/disyn-lv2`.
- Provide UI for selecting algorithm and editing parameters (Attack, Decay, Reverb Size, Reverb Level).

## Assumptions
- Use PlatformIO in VS Code as the build environment.
- UI/DSP split mirrors `/home/danny/github/dreamachine` architecture.
- Use Adafruit OLED library with SSH1106 support; verify exact controller/variant on hardware.
- Target 44.1kHz sample rate if performance and I/O allow; validate on device.

## Phase 0: Project Setup
1. Initialize PlatformIO project for ESP32 DevKit V1 (DOIT). (done)
2. Add dependencies for:
   - I2C OLED (Adafruit_SH110X preferred; fall back to Adafruit_SSD1306 if required by hardware).
   - Encoder handling (custom GPIO-based). (done)
   - Audio/DSP utilities (if required by ported algorithms). (partial; I2S DAC in HAL)
3. Create basic folder layout: `src/`, `include/`, `lib/` for reusable modules. (done)
4. Add a compile-time option for sample rate (default 44.1kHz) to allow quick testing. (done)

## Phase 1: Hardware Abstraction Layer (HAL)
1. GPIO mapping constants from requirements. (done)
2. Drivers:
   - OLED display (init, clear, render text/menus). (done)
   - Encoder (rotation + pushbutton with debouncing). (done)
   - Gate In (digital input, debounce if needed). (done)
   - Gate Out (digital output). (done)
   - ADC inputs for CV0-2 and Pot0-2 (calibration/scaling). (hooks added; tune pending)
   - DAC outputs (write audio samples to DAC1/DAC2). (via I2S DAC; direct DAC driver not used)
   - I2S DAC audio output wrapper. (done)
3. Add basic diagnostics screen showing live input values. (done; Status page + serial change log)
4. Add audio init error indication in status. (done)

## Phase 2: UI Core
1. Menu model for:
   - Algorithm
   - Attack
   - Decay
   - Reverb Size
   - Reverb Level
   - Param 1/2 (algorithm-specific labels)
   - Master Gain
   - Status
2. Encoder switch cycles selection; encoder rotation edits value. (done)
3. Define parameter ranges and default values. (done)
4. Render UI to OLED (simple list with highlight). (done)
5. Support scrolling or paging for algorithm lists longer than screen height. (done)
6. Use abbreviated names where needed for OLED fit. (done)

## Phase 3: DSP Core
1. Port/reference all DSP algorithms from `reference/disyn-lv2`, targeting the full `AlgorithmType` set (0-18). (done)
2. Define DSP parameter interface matching UI parameters. (done)
3. Implement envelope behavior where Attack=0 and Decay=0 yields maximum continuous output. (done)
4. Implement audio processing loop on the DSP core:
   - Read CV/pots as modulation inputs.
   - Update parameters at control rate.
   - Output to DAC1/DAC2.
5. Validate real-time performance and buffer scheduling at 44.1kHz; lower to 32kHz or 22.05kHz if needed. (pending)
6. If CPU is tight, add compile-time switches to drop the heaviest algorithms while keeping the core set. (pending)
7. Add a TEST algorithm for hardware validation (tone generator + input change reporting). (done)

## Algorithm Inventory (from `AlgorithmType.hpp`)
0. Dirichlet Pulse
1. DSF Single
2. DSF Double
3. Tanh Square
4. Tanh Saw
5. PAF
6. Mod FM
7. Combination 1 Hybrid Formant
8. Combination 2 Cascaded
9. Combination 3 Parallel Bank
10. Combination 4 Feedback
11. Combination 5 Morphing
12. Combination 6 Inharmonic
13. Combination 7 Adaptive Filter
14. Novel 1 Multistage
15. Novel 2 Freq Asymmetry
16. Novel 3 Cross Mod
17. Novel 4 Taylor
18. Trajectory
19. TEST (tone + diagnostics)

## Phase 4: Dual-Core Integration
1. Assign UI task to one core, DSP task to the other. (done)
2. Set up thread-safe parameter sharing (lock-free queue or atomic struct). (done; queues)
3. Ensure timing separation (UI refresh vs DSP audio loop). (partial; UI task uses delay, DSP uses I2S buffer)

## Phase 5: Calibration & Tuning
1. ADC scaling for CV and pot ranges. (hooks in place; tune pending)
2. DAC output scaling for audio range. (basic scaling + soft clip; calibration pending)
3. Encoder sensitivity and UI refresh tuning. (pending)
4. Map CV inputs to algorithm parameters (e.g., CV0→Param1, CV1→Param2, CV2→pitch/mod) with smoothing. (done)
5. Decide CV/pot modulation routing for master/reverb/test parameters. (hooks in `Config.h`; tune pending)

## Phase 6: Testing & Bring-up
1. Hardware smoke test for each IO. (pending)
2. UI navigation test. (pending)
3. DSP output test with known input. (pending)
4. Sample-rate validation (CPU load, buffer underruns, DAC stability). (pending)
5. Full system soak test. (pending)

## Deliverables
- `src/main.cpp` with core task scheduling.
- `src/ui/` module (display, menu, encoder input).
- `src/dsp/` module (algorithms, processing loop).
- `src/hal/` module (pins, ADC/DAC, IO drivers).
- `docs/PLAN.md` (this plan).

## Open Questions
- Confirm which Adafruit SH1106/SH1107 library variant compiles cleanly on ESP32.
- Validate 44.1kHz sample rate vs. CPU budget and DAC throughput on the DevKit V1.
- Confirm any algorithm exclusions if the full `AlgorithmType` set proves too heavy.
 - Decide whether TEST algorithm should be gated or always-on when selected.
