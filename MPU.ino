
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <math.h>

MPU6050 mpu(Wire);

// =====================================
// L298N MOTOR PINS
// =====================================

// RIGHT MOTOR DRIVER

// FRONT RIGHT
#define FR_IN1 3
#define FR_IN2 2

// BACK RIGHT
#define BR_IN1 4
#define BR_IN2 5

// LEFT MOTOR DRIVER

// BACK LEFT
#define BL_IN1 9
#define BL_IN2 8

// FRONT LEFT
#define FL_IN1 11
#define FL_IN2 10

// =====================================
// MOTOR DIRECTIONS
// =====================================
// Change individual values to -1
// if physical forward direction is wrong.

const int FR_FORWARD = 1;
const int BR_FORWARD = 1;
const int BL_FORWARD = 1;
const int FL_FORWARD = 1;

// =====================================
// SPEED SETTINGS
// =====================================

const int BASE_SPEED = 170;
const int MAX_SPEED = 230;
const int MIN_SPEED = 90;

// =====================================
// MPU SETTINGS
// =====================================

float targetHeading = 0;

const float DEADBAND = 1.5;
float Kp = 5.0;

// Reverse to +1 if correction
// increases heading error.
const int CORRECTION_SIGN = -1;

// =====================================
// MOVEMENT SELECTION
// =====================================

enum Movement {
  STOP,
  FORWARD,
  BACKWARD,
  STRAFE_LEFT,
  STRAFE_RIGHT
};

// Change this to select movement.
Movement currentMovement = FORWARD;

// =====================================
// MOTOR CONTROL USING ANALOGWRITE
// =====================================

void motor(int pin1, int pin2, int speedVal) {

  speedVal = constrain(speedVal, -255, 255);

  if (speedVal > 0) {
    analogWrite(pin1, speedVal);
    analogWrite(pin2, 0);
  }
  else if (speedVal < 0) {
    analogWrite(pin1, 0);
    analogWrite(pin2, -speedVal);
  }
  else {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
  }
}

// =====================================
// SET ALL FOUR MOTORS
// =====================================

void setMotors(int fr, int br, int bl, int fl) {

  motor(FR_IN1, FR_IN2, fr * FR_FORWARD);
  motor(BR_IN1, BR_IN2, br * BR_FORWARD);

  motor(BL_IN1, BL_IN2, bl * BL_FORWARD);
  motor(FL_IN1, FL_IN2, fl * FL_FORWARD);
}

// =====================================
// STOP ALL MOTORS
// =====================================

void stopAll() {
  setMotors(0, 0, 0, 0);
}

// =====================================
// FORWARD
// =====================================

void forward() {

  setMotors(
    BASE_SPEED,
    BASE_SPEED,
    BASE_SPEED,
    BASE_SPEED
  );
}

// =====================================
// BACKWARD
// =====================================

void backward() {

  setMotors(
    -BASE_SPEED,
    -BASE_SPEED,
    -BASE_SPEED,
    -BASE_SPEED
  );
}

// =====================================
// STRAFE LEFT
// =====================================

void strafeLeft() {

  setMotors(
    BASE_SPEED,
    -BASE_SPEED,
    BASE_SPEED,
    -BASE_SPEED
  );
}

// =====================================
// STRAFE RIGHT
// =====================================

void strafeRight() {

  setMotors(
    -BASE_SPEED,
    BASE_SPEED,
    -BASE_SPEED,
    BASE_SPEED
  );
}

// =====================================
// NORMALIZE ANGLE
// =====================================

float normalizeAngle(float angle) {

  while (angle > 180) angle -= 360;
  while (angle < -180) angle += 360;

  return angle;
}

// =====================================
// MPU FORWARD HEADING HOLD
// =====================================

void forwardWithMPU() {

  float currentHeading = mpu.getAngleZ();

  float error = normalizeAngle(
    targetHeading - currentHeading
  );

  float correction = 0;

  if (fabs(error) > DEADBAND) {
    correction = Kp * error * CORRECTION_SIGN;
  }

  correction = constrain(
    correction,
    -70.0f,
    70.0f
  );

  int leftSpeed = constrain(
    (int)(BASE_SPEED + correction),
    MIN_SPEED,
    MAX_SPEED
  );

  int rightSpeed = constrain(
    (int)(BASE_SPEED - correction),
    MIN_SPEED,
    MAX_SPEED
  );

  setMotors(
    rightSpeed,
    rightSpeed,
    leftSpeed,
    leftSpeed
  );

  // SERIAL MONITOR
  static unsigned long lastPrint = 0;

  if (millis() - lastPrint >= 100) {

    lastPrint = millis();

    Serial.print("Target: ");
    Serial.print(targetHeading);

    Serial.print(" | Yaw: ");
    Serial.print(currentHeading);

    Serial.print(" | Error: ");
    Serial.print(error);

    Serial.print(" | Left: ");
    Serial.print(leftSpeed);

    Serial.print(" | Right: ");
   Serial.println(rightSpeed);
  }
}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  pinMode(FR_IN1, OUTPUT);
  pinMode(FR_IN2, OUTPUT);

  pinMode(BR_IN1, OUTPUT);
  pinMode(BR_IN2, OUTPUT);

  pinMode(BL_IN1, OUTPUT);
  pinMode(BL_IN2, OUTPUT);

  pinMode(FL_IN1, OUTPUT);
  pinMode(FL_IN2, OUTPUT);

  stopAll();

  // Arduino Mega I2C
  // SDA = 20
  // SCL = 21

  Wire.begin();

  byte status = mpu.begin();

  if (status != 0) {

    Serial.println("MPU6050 CONNECTION ERROR!");

    while (1) {
      stopAll();
      delay(100);
    }
  }

  Serial.println("Keep robot stationary");
  Serial.println("Calibrating MPU6050...");

  delay(1000);

  mpu.calcOffsets(true, true);

  mpu.update();

  targetHeading = mpu.getAngleZ();

  Serial.print("Target Heading: ");
  Serial.println(targetHeading);

  Serial.println("Starting movement test");

  delay(2000);
}

// =====================================
// MAIN LOOP
// =====================================

void loop() {

  mpu.update();

  switch (currentMovement) {

    case FORWARD:
      forwardWithMPU();
      break;

    case BACKWARD:
      backward();
      break;

    case STRAFE_LEFT:
      strafeLeft();
      break;

    case STRAFE_RIGHT:
      strafeRight();
      break;

    default:
      stopAll();
      break;
  }
}
