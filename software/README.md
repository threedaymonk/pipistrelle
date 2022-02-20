# Software for the Pipistrelle

All software is developed using [PlatformIO](https://platformio.org/) with the
Arduino toolchain for the Seeeduino Xiao.

# Calibration

To initiate calibration, turn all the pots to minimum, connect a patch cable
from **Out** to **CV1**, and power on the module.

The LED flashes.

Send 1V from a known source to **V/oct** and turn **A** fully clockwise.

The LED flashes twice.

Send 3V from a known source to **V/oct** and turn **B** fully clockwise.

The oscillator will now boot as normal.

## X-Y Oscillator

This oscillator generates sine, triangle, saw, and square waves and blends
between them using the C and D pots and CV inputs.

- **Pot A**: Coarse tune (C0 + 6 octaves)
- **Pot B**: Fine tune (+/- 1/2 octave)
- **Pot C + CV 1**: X
- **Pot D + CV 2**: Y

X and Y morph between waveforms thus:

     sine  --> triangle
      |           |
      v           v
    square -->   saw

## Supersaw

7 detuned saw waves and a square wave sub-oscillator an octave below

- **Pot A**: Coarse tune (C0 + 6 octaves)
- **Pot B**: Fine tune (+/- 1/2 octave)
- **Pot C + CV 1**: Detune amount
- **Pot D + CV 2**: Sub oscillator level

## Wavetable

Wavetable oscillator panning around 64 256-sample wavetables in an 8 by 8
plane.

- **Pot A**: Coarse tune (C0 + 6 octaves)
- **Pot B**: Fine tune (+/- 1/2 octave)
- **Pot C + CV 1**: X
- **Pot D + CV 2**: Y
