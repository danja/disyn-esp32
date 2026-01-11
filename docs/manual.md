# Disyn ESP32 Manual

## Overview
Disyn ESP32 is a Eurorack module built around an ESP32 DevKit V1. It runs a dual-core firmware: the UI (display/encoder) on one core and DSP (audio algorithms) on the other. Algorithms are derived from the Disyn LV2 reference set, plus a TEST mode for calibration and diagnostics.

## Controls
- **Encoder rotate**: Adjusts the current parameter value.
- **Encoder press**: Cycles to the next parameter.
- **Gate In**: Triggers the envelope; when Attack=0 and Decay=0, the output is continuous at max level.

## UI Pages
The OLED menu shows a short list with the current selection indicated by `>`.

Menu entries (abbreviated for OLED fit):
- **Alg**: Algorithm selection.
- **Atk**: Attack (0.00–1.00).
- **Dec**: Decay (0.00–1.00).
- **Rev Sz**: Reverb size (0.00–1.00).
- **Rev Lv**: Reverb level (0.00–1.00).
- **P1**: Algorithm parameter 1 (label changes per algorithm).
- **P2**: Algorithm parameter 2 (label changes per algorithm).
- **Mast**: Master gain (0.00–1.00).
- **Stat**: Status/diagnostics.

## Algorithms (Abbreviated)
- **Dir Pulse**: Dirichlet Pulse (Harm, Tilt)
- **DSF S**: DSF Single (Dec, Rat)
- **DSF D**: DSF Double (Dec, Rat)
- **Tanh Sq**: Tanh Square (Drv, Trim)
- **Tanh Saw**: Tanh Saw (Drv, Blend)
- **PAF**: Phase Aligned Formant (Form, BW)
- **Mod FM**: Modified FM (Idx, Rat)
- **C1 Hyb**: Combo 1 Hybrid Formant (Idx)
- **C2 Cas**: Combo 2 Cascaded (DSF Dec, Asym)
- **C3 Par**: Combo 3 Parallel Bank (Idx)
- **C4 Fdb**: Combo 4 Feedback (Idx, Fb)
- **C5 Mor**: Combo 5 Morphing (Morph, Char)
- **C6 Inh**: Combo 6 Inharmonic (DSF Dec, PAF Sh)
- **C7 Flt**: Combo 7 Adaptive Filter (Cut, Res)
- **N1 Mul**: Novel 1 Multistage (Tanh, Exp)
- **N2 Asy**: Novel 2 Frequency Asym (LowR, HiR)
- **N3 XMod**: Novel 3 Cross Mod (M1, M2)
- **N4 Tay**: Novel 4 Taylor (T1, T2)
- **Traj**: Trajectory (Sides, Ang)
- **TEST**: Test tone + diagnostics (Freq, Level)

## CV/Pot Modulation (Default)
- **CV0/Pot0** → Param 1 (normalized, moderate depth)
- **CV1/Pot1** → Param 2 (normalized, moderate depth)
- **CV2/Pot2** → Pitch (dominant)
- **CV2/Pot2** → Reverb size/level (light), Master gain (very light)

These amounts are defined in `include/Config.h` and can be tuned.

## Status Page
The Status page shows:
- Underrun count (I2S DAC buffer short writes)
- Live CV/Pot values with change markers

If audio init fails, the Status page shows `AUD FAIL`.

## Calibration Mode (TEST + Status)
When **Alg = TEST** and **Stat** is selected, the status page switches to calibration view:
- Cycles through inputs once per second (C0/C1/C2/P0/P1/P2)
- Shows raw **Cur / Min / Max** ADC values
- Long-press the encoder (>1s) to reset min/max (OLED shows `CAL RESET`, serial prints it)

## Hardware Bring-Up Checklist
1. **Power**
   - Confirm +5V and +3.3V rails are stable.
   - Verify grounds are common for ESP32, display, and buffers.
2. **Display**
   - If OLED is blank, verify I2C wiring and address (0x3C typical).
   - Confirm library variant (SH1106 vs SH1107) in `platformio.ini`.
3. **Encoder**
   - Check rotation increments and button press detection.
   - Verify no stuck button due to wiring/pullups.
4. **Gate IO**
   - Verify Gate In changes (use a square wave or manual signal).
   - Gate Out should mirror Gate In.
5. **ADC Inputs**
   - Set **Alg = TEST**, go to **Stat**.
   - Sweep CV/Pots and confirm raw min/max update.
   - Set calibration in `include/Calibration.h` based on observed ranges.
6. **Audio Output**
   - Start with **Mast** low.
   - In **TEST**, set Freq to ~440Hz and Level low.
   - Listen for clean tone; adjust Master Gain as needed.
7. **DSP**
   - Switch to a non-TEST algorithm.
   - Confirm Gate triggers sound; Attack/Decay shape the envelope.
8. **Performance**
   - Watch Status underruns over several minutes.
   - If underruns grow, lower `DISYN_SAMPLE_RATE` and rebuild.

## Notes
- Attack=0 and Decay=0 forces continuous output at full envelope level.
- Param labels change with algorithm; “Unused” parameters show `-`.
- Use TEST mode for calibration before musical operation.
