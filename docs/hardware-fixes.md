# Hardware Fixes

Rewiring required to make the CV, pot and gate inputs work. The firmware in this
repo already expects the **new** pinout below, so the board must be rewired
before the inputs will read correctly.

## Summary of moves

| Signal | Old pin | New pin | Reason |
|---|---|---|---|
| Encoder CLK | 34 | **14** | Frees ADC1_CH6 for a pot; 14 has an internal pull-up |
| Encoder DT | 35 | **13** | Frees ADC1_CH7 for a pot; 13 has an internal pull-up |
| Pot1 | 27 | **34** | Gets Pot1 off ADC2 |
| Pot2 | 4 | **35** | Gets Pot2 off ADC2 |
| Pot0 | 33 | 33 (unchanged) | Firmware was wrong, not the wiring — see below |

Unchanged: DAC1 25, DAC2 26, SDA 21, SCL 22, Encoder SW 16, CV0 36, CV1 39,
CV2 32, Gate In 18, Gate Out 19.

GPIO 27 and 4 are now free.

## Why

### 1. The pots were on ADC2, which cannot work on this board

Audio output uses I2S in `I2S_MODE_DAC_BUILT_IN` on GPIO25/26
(`src/hal/AudioOutput.cpp`). Those pins are `ADC2_CHANNEL_8` and
`ADC2_CHANNEL_9`, and the I2S DMA drives that RTC/SAR block continuously at the
audio sample rate. Any `adc2_get_raw()` read has to arbitrate for the same
block, so ADC2 reads come back dead or intermittent for as long as audio is
running.

Pot1 (GPIO27 = ADC2_CH7) and Pot2 (GPIO4 = ADC2_CH0) were both on ADC2. Moving
them to 34/35 puts all six analog inputs on ADC1.

The DevKit V1 only breaks out six of ADC1's eight channels — 32, 33, 34, 35, 36,
39 — and 37/38 are not bonded out. Six analog inputs therefore need all six, so
the encoder had to move off 34/35. That is the whole reason for the encoder
change; nothing was wrong with the encoder itself.

### 2. Pot0 and Pot2 were swapped in firmware

`include/PinConfig.h` had `kPinPot0 = 4` and `kPinPot2 = 33`, but the wiring
spec had Pot0 on 33 and Pot2 on 4. Commit `253ed2e` ("tweaks") introduced the
swap; `473c3a1` had it right.

The effect: turning physical Pot0 moved `params.pot2` (pitch), and turning
physical Pot2 moved `params.pot0` (wavefolder amount). Pot0 looked like it
worked, Pot2 looked dead. This is now fixed in firmware — **do not rewire Pot0**,
it stays on 33.

### 3. Gate In floated

`src/hal/Gate.cpp` set the input to plain `INPUT` while `Gate::read()` treats
LOW as gate-asserted. An unpatched jack floats, and a floating LOW reads as a
stuck or randomly firing trigger. It now uses `INPUT_PULLUP`, so idle reads HIGH
= gate off.

## Pins to avoid when changing anything else

- **12 (MTDI)** and **15 (MTDO)** are strapping pins — do not use for inputs
  with pull-ups, they affect boot.
- **0** and **2** are strapping pins used for flashing.
- **6-11** are the SPI flash, unusable.
- **34, 35, 36, 39** are input-only and have **no internal pull-ups**. Fine for
  pot wipers and buffered CV, not for switches or encoders without external
  pull-ups.
- Anything on **ADC2** (0, 2, 4, 12, 13, 14, 15, 25, 26, 27) is unusable as an
  analog input while audio is running.

## Bring-up checks after rewiring

1. **Encoder** — rotate and press. If rotation is backwards, swap the wires on
   14 and 13 (or swap `kPinEncClk`/`kPinEncDt`).
2. **Pots** — select TEST algorithm, go to the Status page. It cycles through
   C0/C1/C2/P0/P1/P2 showing Cur/Min/Max. Sweep each pot and confirm the
   matching channel moves over roughly the full 0-4095 range.
3. **CVs** — same page. CV0/CV1/CV2 are on 36/39/32 and were never miswired, so
   if they still do not move, the problem is the input buffers or the jacks, not
   the firmware.
4. **Gate In** — with nothing patched, the envelope should stay idle. Patch a
   gate and confirm it triggers. If it is inverted (triggers when the gate goes
   away), the input buffer is non-inverting and `Gate::read()` needs its `== LOW`
   changed to `== HIGH`.
5. **Gate Out** — should go low while sound is playing, high when silent.

## Gotcha that looks like a hardware fault

If **Attack and Decay are both 0**, `src/dsp/DspTask.cpp` sets
`forceContinuous`, which makes the engine gate permanently true. Gate In is then
ignored entirely and Gate Out sticks low. This is intended behaviour (a drone
mode) but it looks exactly like a dead gate input — check the envelope settings
before suspecting the wiring.
