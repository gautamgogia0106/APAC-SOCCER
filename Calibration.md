# APDS9960 colour calibration

The main `FINAL.ino` and the separately uploaded `color_threshold.ino` use **different** thresholds. Both sets are preserved as supplied; the test sketch is not silently modified to match the main firmware.

## TCA9548A channels

| Position | Channel |
|---|---:|
| Front | 6 |
| Back | 1 |
| Left | 2 |
| Right | 0 |

## Main firmware thresholds

The firmware computes `RG = red * 1000 / green` and `BG = blue * 1000 / green` and classifies white when both thresholds are met.

| Position | RG minimum | BG minimum |
|---|---:|---:|
| Front | 675 | 735 |
| Back | 630 | 570 |
| Left | 760 | 755 |
| Right | 735 | 715 |

## Standalone colour-threshold test thresholds

| Position | RG minimum | BG minimum |
|---|---:|---:|
| Front | 625 | 620 |
| Back | 735 | 650 |
| Left | 690 | 590 |
| Right | 800 | 620 |

Calibrate under actual competition lighting and verify white/green readings before driving.
