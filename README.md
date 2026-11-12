# RoboCup Soccer 2v2 Attacker Robot

Arduino Mega 2560 autonomous soccer attacker project with four-wheel movement, MPU6050 heading correction, four APDS9960 colour sensors through a TCA9548A I2C multiplexer, and HiTechnic IR Seeker V2 ball tracking.

> **Safety:** The main firmware in `firmware/RoboCup_Attacker/` has `ENABLE_MOTORS = true` by default. The unmodified uploaded `FINAL.ino` is preserved in `originals/` and **has motors enabled**. Other motor test sketches can move the motors when uploaded. Lift wheels off the ground during tests.

## Repository contents

| Path | Purpose |
|---|---|
| `firmware/RoboCup_Attacker/RoboCup_Attacker.ino` | Main firmware, motors disabled by default |
| `originals/FINAL.ino` | Unmodified original main firmware (motors enabled) |
| `tests/INDIVIDUAL_MOTOR/` | Individual motor test |
| `tests/MOVEMENT/` | Movement test |
| `tests/MPU/` | MPU and movement test |
| `tests/color_threshold/` | APDS9960 colour threshold test |
| `docs/Wiring.md` | Pin and I2C address reference |
| `docs/Calibration.md` | Sensor mapping and threshold notes |

## Requirements

- Arduino IDE with **Arduino Mega 2560** selected
- `MPU6050_light` Arduino library for sketches that include it
- `Wire` library (included with Arduino AVR core)
- Hardware: Arduino Mega, MPU6050, TCA9548A, four APDS9960 sensors, HiTechnic IR Seeker V2, motor drivers and four motors

## Getting started

1. Open `firmware/RoboCup_Attacker/RoboCup_Attacker.ino` in Arduino IDE.
2. Install `MPU6050_light` through Library Manager.
3. Select **Tools → Board → Arduino Mega or Mega 2560**, then select the correct port.
4. **Motors are enabled by default.** For sensor-only testing, set `ENABLE_MOTORS = false` before uploading.
5. Upload and open Serial Monitor at **115200 baud**.
6. Verify sensor readings and power wiring before enabling motors.

Each `.ino` sketch is in a folder with the same basename, so Arduino IDE can open it independently.

## Project status

Development and hardware testing. Sensor calibration and motor behavior require verification on the actual robot. The sketches are preserved from the supplied source files; no compile or hardware validation is claimed.

## License

No license is included yet. Choose a license before inviting outside reuse; without one, standard copyright restrictions apply.
