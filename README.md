# Disyn ESP32

**Status:** Untested on hardware. Expect breakage and adjust calibration before first use.

## Overview
Disyn ESP32 is a Eurorack module built around an ESP32 DevKit V1. It runs the UI on one core and DSP on the other, using algorithms derived from the Disyn LV2 reference set. A TEST algorithm is included for calibration and diagnostics.

## Features
- Dual-core UI/DSP split
- 19 Disyn algorithms + TEST tone/diagnostics
- SSH1106/SH1107 OLED UI with encoder control
- CV/pot modulation routing for params and pitch
- I2S DAC audio output

## Hardware
- ESP32 DevKit V1 (DOIT)
- SH1106-based 1.3" I2C OLED
- Rotary encoder with pushbutton
- Gate In / Gate Out
- CV0/CV1/CV2 inputs
- Pot0/Pot1/Pot2 inputs
- DAC1/DAC2 outputs

Pin mapping is in `include/PinConfig.h`, and the full wiring table is in
`docs/requirements.md`.

**Rewiring required:** the encoder and Pot1/Pot2 have moved pins so that all six
analog inputs sit on ADC1 — ADC2 cannot be read while the I2S built-in DAC is
running. See `docs/hardware-fixes.md` before flashing.

## Build
This project uses PlatformIO.

```bash
pio run
```

Set sample rate with `DISYN_SAMPLE_RATE` in `platformio.ini` or `include/Config.h`.

## Flashing
Connect the ESP32 DevKit V1 to the machine over USB, then build and write the firmware:

```bash
pio run --target upload
```

PlatformIO auto-detects the serial port. To pick one explicitly:

```bash
pio device list
pio run --target upload --upload-port /dev/ttyUSB0
```

On Linux, serial access needs the PlatformIO udev rules and membership of the
`dialout` group (log out and back in afterwards):

```bash
sudo cp 99-platformio-udev.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
sudo usermod -aG dialout $USER
```

If upload fails to sync, hold the BOOT button while the upload starts and release
once it begins writing. To watch the serial output afterwards:

```bash
pio device monitor
```

The monitor runs at 115200 baud (`monitor_speed` in `platformio.ini`). If the device
is in a bad state, erase it with `pio run --target erase` before uploading again.

## Usage
- Rotate encoder for values
- Press encoder to move between parameters
- Use TEST algorithm for calibration and input diagnostics

Full instructions and a bring-up checklist are in `docs/manual.md`.

## Calibration
Raw ADC min/max tracking and normalization are configurable in `include/Calibration.h`. Use TEST + Status to observe raw ranges and update calibration constants.

## Reference
DSP is based on the `reference/disyn-lv2` algorithms.

## Notes
- Attack=0 and Decay=0 force continuous maximum output.
- If you see underruns on the Status page, lower `DISYN_SAMPLE_RATE` and rebuild.
