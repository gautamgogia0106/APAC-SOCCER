# Wiring reference

Based on the uploaded Arduino sketches. **Verify all wiring on your actual robot before powering motors.**

## Arduino Mega motor signal pins

| Wheel | IN1 | IN2 |
|---|---:|---:|
| Front right | 3 | 2 |
| Back right | 4 | 5 |
| Back left | 9 | 8 |
| Front left | 11 | 10 |

The sketches use PWM via `analogWrite` for the motor inputs. Motor-driver power, enable connections, ground and supply voltage must be checked separately.

## I2C

- Arduino Mega SDA: pin 20
- Arduino Mega SCL: pin 21
- TCA9548A address: `0x70` (with address pins at default setting)
- APDS9960 address: `0x39` on selected TCA channel
- HiTechnic IR Seeker V2 address in main sketch: `0x08`
- MPU6050 connected via I2C (address determined by hardware configuration)

See `Calibration.md` for channel mapping. Ensure all modules use compatible logic voltage and share ground. Do not power motors directly from Mega pins.
