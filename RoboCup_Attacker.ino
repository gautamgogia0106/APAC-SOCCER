
#include <Arduino.h>
#include <Wire.h>
#include <MPU6050_light.h>
#include <math.h>

// =====================================
// ROBOCUP SOCCER 2V2 ATTACKER
// ARDUINO MEGA 2560
// 4 APDS9960 + TCA9548A
// MPU6050 + HITECHNIC IR SEEKER V2
// =====================================

const bool ENABLE_MOTORS = true;

// =====================================
// MOTOR PINS
// =====================================

#define FR_IN1 3
#define FR_IN2 2

#define BR_IN1 4
#define BR_IN2 5

#define BL_IN1 9
#define BL_IN2 8

#define FL_IN1 11
#define FL_IN2 10

const int FR_FORWARD = 1;
const int BR_FORWARD = 1;
const int BL_FORWARD = 1;
const int FL_FORWARD = 1;

// =====================================
// SPEED SETTINGS
// =====================================

const int APPROACH_SPEED = 165;
const int STRAFE_SPEED   = 185;
const int ATTACK_SPEED   = 220;
const int ESCAPE_SPEED   = 190;
const int MAX_SPEED      = 230;

// =====================================
// MPU6050
// =====================================

MPU6050 mpu(Wire);

float targetHeading = 0;

const float HEADING_KP = 5.0;
const float HEADING_DEADBAND = 1.5;

const int MAX_CORRECTION = 55;
const int CORRECTION_SIGN = -1;

// =====================================
// IR SEEKER
// =====================================

#define IR_ADDRESS 0x08
#define IR_DIRECTION_REG 0x49

int ballDirection = 0;

// =====================================
// TCA9548A + APDS9960
// =====================================

#define TCA_ADDR  0x70
#define APDS_ADDR 0x39

#define FRONT 0
#define BACK  1
#define LEFT  2
#define RIGHT 3

const byte CHANNELS[4] = {
  6,  // FRONT
  1,  // BACK
  2,  // LEFT
  0   // RIGHT
};

const char* SENSOR_NAMES[4] = {
  "FRONT",
  "BACK",
  "LEFT",
  "RIGHT"
};

// =====================================
// CALIBRATED WHITE THRESHOLDS
// =====================================

// RED / GREEN * 1000
const int WHITE_RG_MIN[4] = {
  675,  // FRONT
  630,  // BACK
  760,  // LEFT
  735   // RIGHT
};

// BLUE / GREEN * 1000
const int WHITE_BG_MIN[4] = {
  735,  // FRONT
  570,  // BACK
  755,  // LEFT
  715   // RIGHT
};

// =====================================
// APDS9960 REGISTERS
// =====================================

#define APDS_ENABLE  0x80
#define APDS_ATIME   0x81
#define APDS_CONTROL 0x8F
#define APDS_ID      0x92
#define APDS_STATUS  0x93

#define APDS_CDATAL  0x94
#define APDS_RDATAL  0x96
#define APDS_GDATAL  0x98
#define APDS_BDATAL  0x9A

// =====================================
// COLOR SENSOR DATA
// =====================================

struct ColorData {
  uint16_t clear;
  uint16_t red;
  uint16_t green;
  uint16_t blue;

  int rgRatio;
  int bgRatio;

  bool white;
  bool valid;
};

ColorData color[4];

bool sensorReady[4] = {
  false, false, false, false
};

unsigned long colorUpdated[4] = {
  0, 0, 0, 0
};

byte nextColorSensor = 0;

// =====================================
// TIMING
// =====================================

const unsigned long COLOR_INTERVAL = 15;
const unsigned long COLOR_FRESH_MS = 250;

const unsigned long ESCAPE_TIME = 200;
const unsigned long DEBUG_INTERVAL = 250;
const unsigned long START_DELAY = 3000;

unsigned long lastColorRead = 0;
unsigned long lastDebug = 0;

unsigned long escapeUntil = 0;
unsigned long whiteClearedAt = 0;

bool robotEnabled = false;
bool escapeActive = false;

// =====================================
// ESCAPE MODES
// =====================================

enum EscapeMode {
  ESCAPE_NONE,
  ESCAPE_FORWARD,
  ESCAPE_BACKWARD,
  ESCAPE_LEFT,
  ESCAPE_RIGHT,
  ESCAPE_FORWARD_LEFT,
  ESCAPE_FORWARD_RIGHT,
  ESCAPE_BACKWARD_LEFT,
  ESCAPE_BACKWARD_RIGHT
};

EscapeMode escapeMode = ESCAPE_NONE;

// =====================================
// MOTOR CONTROL
// =====================================

void motor(int pin1, int pin2, int speedVal) {

  if (!ENABLE_MOTORS) {
    analogWrite(pin1, 0);
    analogWrite(pin2, 0);
    return;
  }

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

void setMotors(int fr, int br, int bl, int fl) {

  motor(FR_IN1, FR_IN2, fr * FR_FORWARD);
  motor(BR_IN1, BR_IN2, br * BR_FORWARD);
  motor(BL_IN1, BL_IN2, bl * BL_FORWARD);
  motor(FL_IN1, FL_IN2, fl * FL_FORWARD);
}

void stopMotors() {
  setMotors(0, 0, 0, 0);
}

// =====================================
// MPU6050 HEADING CORRECTION
// =====================================

float normalizeAngle(float angle) {

  while (angle > 180) angle -= 360;
  while (angle < -180) angle += 360;

  return angle;
}

int getCorrection() {

  float error = normalizeAngle(
    targetHeading - mpu.getAngleZ()
  );

  if (fabs(error) < HEADING_DEADBAND)
    return 0;

  int correction = (int)(
    error * HEADING_KP * CORRECTION_SIGN
  );

  return constrain(
    correction,
    -MAX_CORRECTION,
    MAX_CORRECTION
  );
}

// =====================================
// OMNI WHEEL MOVEMENT
// =====================================

void moveRobot(int forwardSpeed, int strafeSpeed) {

  int correction = getCorrection();

  int fr = forwardSpeed - strafeSpeed - correction;
  int br = forwardSpeed + strafeSpeed - correction;
  int bl = forwardSpeed + strafeSpeed + correction;
  int fl = forwardSpeed - strafeSpeed + correction;

  int peak = max(
    max(abs(fr), abs(br)),
    max(abs(bl), abs(fl))
  );

  if (peak > MAX_SPEED) {

    fr = (long)fr * MAX_SPEED / peak;
    br = (long)br * MAX_SPEED / peak;
    bl = (long)bl * MAX_SPEED / peak;
    fl = (long)fl * MAX_SPEED / peak;
  }

  setMotors(fr, br, bl, fl);
}

void forward(int speedVal) {
  moveRobot(speedVal, 0);
}

void backward(int speedVal) {
  moveRobot(-speedVal, 0);
}

void strafeLeft(int speedVal) {
  moveRobot(0, -speedVal);
}

void strafeRight(int speedVal) {
  moveRobot(0, speedVal);
}

void forwardLeft(int speedVal) {
  moveRobot(speedVal, -speedVal);
}

void forwardRight(int speedVal) {
  moveRobot(speedVal, speedVal);
}

void backwardLeft(int speedVal) {
  moveRobot(-speedVal, -speedVal);
}

void backwardRight(int speedVal) {
  moveRobot(-speedVal, speedVal);
}

// =====================================
// TCA9548A
// =====================================

bool selectChannel(byte channel) {

  Wire.beginTransmission(TCA_ADDR);
  Wire.write((byte)0);

  if (Wire.endTransmission() != 0)
    return false;

  Wire.beginTransmission(TCA_ADDR);
  Wire.write((byte)(1 << channel));

  return Wire.endTransmission() == 0;
}

bool disableTCAChannels() {

  Wire.beginTransmission(TCA_ADDR);
  Wire.write((byte)0);

  return Wire.endTransmission() == 0;
}

// =====================================
// APDS REGISTER FUNCTIONS
// =====================================

bool writeAPDS(byte reg, byte value) {

  Wire.beginTransmission(APDS_ADDR);
  Wire.write(reg);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

int readAPDS(byte reg) {

  Wire.beginTransmission(APDS_ADDR);
  Wire.write(reg);

  if (Wire.endTransmission(false) != 0)
    return -1;

  if (Wire.requestFrom(
      (uint8_t)APDS_ADDR,
      (uint8_t)1) != 1)
    return -1;

  return Wire.read();
}

bool read16(byte reg, uint16_t &value) {

  int low = readAPDS(reg);
  int high = readAPDS(reg + 1);

  if (low < 0 || high < 0)
    return false;

  value = (uint16_t)low |
          ((uint16_t)high << 8);

  return true;
}

// =====================================
// INITIALIZE APDS9960
// =====================================

bool initAPDS(byte index) {

  if (!selectChannel(CHANNELS[index]))
    return false;

  int id = readAPDS(APDS_ID);

  if (id != 0x9E &&
      id != 0xAB &&
      id != 0x9C) {

    Serial.print(SENSOR_NAMES[index]);
    Serial.println(" ID ERROR");

    return false;
  }

  bool ok = true;

  ok &= writeAPDS(APDS_ENABLE, 0x00);
  delay(10);

  ok &= writeAPDS(APDS_ATIME, 0xDB);
  ok &= writeAPDS(APDS_CONTROL, 0x02);
  ok &= writeAPDS(APDS_ENABLE, 0x03);

  delay(50);

  return ok;
}

// =====================================
// WHITE DETECTION
// =====================================

bool detectWhite(
  byte index,
  uint16_t r,
  uint16_t g,
  uint16_t b,
  int &rg,
  int &bg
) {

  if (g == 0) {
    rg = 0;
    bg = 0;
    return false;
  }

  rg = (long)r * 1000L / g;
  bg = (long)b * 1000L / g;

  return (
    rg >= WHITE_RG_MIN[index] &&
    bg >= WHITE_BG_MIN[index]
  );
}

// =====================================
// READ COLOR SENSOR
// =====================================

bool readColor(byte index) {

  if (!selectChannel(CHANNELS[index]))
    return false;

  int status = readAPDS(APDS_STATUS);

  if (status < 0)
    return false;

  // Measurement not yet ready.
  // Keep previous reading if available.
  if (!(status & 0x01))
    return false;

  uint16_t c, r, g, b;

  bool ok =
    read16(APDS_CDATAL, c) &&
    read16(APDS_RDATAL, r) &&
    read16(APDS_GDATAL, g) &&
    read16(APDS_BDATAL, b);

  if (!ok)
    return false;

  int rg, bg;

  bool white = detectWhite(
    index, r, g, b, rg, bg
  );

  color[index].clear = c;
  color[index].red = r;
  color[index].green = g;
  color[index].blue = b;

  color[index].rgRatio = rg;
  color[index].bgRatio = bg;

  color[index].white = white;
  color[index].valid = true;

  colorUpdated[index] = millis();

  return true;
}

// =====================================
// UPDATE COLOR SENSORS
// =====================================

void updateColorSensors() {

  unsigned long now = millis();

  if (now - lastColorRead < COLOR_INTERVAL)
    return;

  lastColorRead = now;

  byte index = nextColorSensor;

  if (sensorReady[index]) {

    // Do not invalidate a previous reading
    // because of one failed attempt.
    // colorsHealthy() checks its age.
    readColor(index);
  }

  nextColorSensor =
    (nextColorSensor + 1) % 4;
}

// =====================================
// CHECK SENSOR HEALTH
// =====================================

bool colorsHealthy() {

  unsigned long now = millis();

  for (byte i = 0; i < 4; i++) {

    if (!sensorReady[i])
      return false;

    if (!color[i].valid)
      return false;

    if (now - colorUpdated[i] > COLOR_FRESH_MS)
      return false;
  }

  return true;
}

// =====================================
// INITIAL COLOR READ WITH RETRIES
// =====================================

bool initializeColorReadings() {

  Serial.println(
    "Waiting for color sensor data..."
  );

  for (byte i = 0; i < 4; i++) {

    bool success = false;

    for (byte attempt = 0; attempt < 10; attempt++) {

      if (readColor(i)) {
        success = true;
        break;
      }

      delay(30);
    }

    if (!success) {

      Serial.print("COLOR READ FAILED: ");
      Serial.println(SENSOR_NAMES[i]);

      return false;
    }

    Serial.print(SENSOR_NAMES[i]);
    Serial.println(" COLOR DATA READY");
  }

  return true;
}

// =====================================
// IR SEEKER V2
// =====================================

int readBallDirection() {

  if (!disableTCAChannels())
    return -1;

  Wire.beginTransmission(IR_ADDRESS);
  Wire.write(IR_DIRECTION_REG);

  if (Wire.endTransmission(false) != 0)
    return -1;

  if (Wire.requestFrom(
      (uint8_t)IR_ADDRESS,
      (uint8_t)1) != 1)
    return -1;

  int direction = Wire.read();

  if (direction < 0 || direction > 9)
    return -1;

  return direction;
}

// =====================================
// WHITE BOUNDARY ESCAPE
// =====================================

void runEscape() {

  switch (escapeMode) {

    case ESCAPE_FORWARD:
      forward(ESCAPE_SPEED);
      break;

    case ESCAPE_BACKWARD:
      backward(ESCAPE_SPEED);
      break;

    case ESCAPE_LEFT:
      strafeLeft(ESCAPE_SPEED);
      break;

    case ESCAPE_RIGHT:
      strafeRight(ESCAPE_SPEED);
      break;

    case ESCAPE_FORWARD_LEFT:
      forwardLeft(ESCAPE_SPEED);
      break;

    case ESCAPE_FORWARD_RIGHT:
      forwardRight(ESCAPE_SPEED);
      break;

    case ESCAPE_BACKWARD_LEFT:
      backwardLeft(ESCAPE_SPEED);
      break;

    case ESCAPE_BACKWARD_RIGHT:
      backwardRight(ESCAPE_SPEED);
      break;

    default:
      stopMotors();
      break;
  }
}

bool avoidBoundary() {

  unsigned long now = millis();

  bool frontWhite = color[FRONT].white;
  bool backWhite = color[BACK].white;
  bool leftWhite = color[LEFT].white;
  bool rightWhite = color[RIGHT].white;

  bool anyWhite =
    frontWhite || backWhite ||
    leftWhite || rightWhite;

  // Opposite boundaries detected:
  // stop rather than choose a direction.
  if ((frontWhite && backWhite) ||
      (leftWhite && rightWhite)) {

    escapeMode = ESCAPE_NONE;
    escapeActive = false;

    stopMotors();
    return true;
  }

  // White boundary detected.
  if (anyWhite) {

    if (frontWhite && leftWhite) {
      escapeMode = ESCAPE_BACKWARD_RIGHT;
    }
    else if (frontWhite && rightWhite) {
      escapeMode = ESCAPE_BACKWARD_LEFT;
    }
    else if (backWhite && leftWhite) {
      escapeMode = ESCAPE_FORWARD_RIGHT;
    }
    else if (backWhite && rightWhite) {
      escapeMode = ESCAPE_FORWARD_LEFT;
    }
    else if (frontWhite) {
      escapeMode = ESCAPE_BACKWARD;
    }
    else if (backWhite) {
      escapeMode = ESCAPE_FORWARD;
    }
    else if (leftWhite) {
      escapeMode = ESCAPE_RIGHT;
    }
    else if (rightWhite) {
      escapeMode = ESCAPE_LEFT;
    }

    escapeActive = true;
    whiteClearedAt = 0;

    runEscape();
    return true;
  }

  // Boundary has cleared.
  // Continue the previous escape for 200 ms.
  if (escapeActive) {

    if (whiteClearedAt == 0)
      whiteClearedAt = now;

    if (now - whiteClearedAt < ESCAPE_TIME) {
      runEscape();
      return true;
    }

    escapeActive = false;
    escapeMode = ESCAPE_NONE;
    whiteClearedAt = 0;
  }

  return false;
}

// =====================================
// ATTACKER BALL TRACKING
// =====================================

void attackerLogic(int direction) {

  switch (direction) {

    case 1:
    case 2:
      strafeLeft(STRAFE_SPEED);
      break;

    case 3:
    case 4:
      forwardLeft(APPROACH_SPEED);
      break;

    case 5:
      forward(ATTACK_SPEED);
      break;

    case 6:
    case 7:
      forwardRight(APPROACH_SPEED);
      break;

    case 8:
    case 9:
      strafeRight(STRAFE_SPEED);
      break;

    case 0:
    default:
      stopMotors();
      break;
  }
}

// =====================================
// SERIAL DEBUG
// =====================================

void printDebug() {

  Serial.print("IR:");
  Serial.print(ballDirection);

  Serial.print(" | YAW:");
  Serial.print(mpu.getAngleZ());

  Serial.print(" | TARGET:");
  Serial.print(targetHeading);

  Serial.print(" | ESCAPE:");
  Serial.print((int)escapeMode);

  for (byte i = 0; i < 4; i++) {

    Serial.print(" | ");
    Serial.print(SENSOR_NAMES[i]);
    Serial.print(":");

    if (!color[i].valid) {
      Serial.print("INVALID");
      continue;
    }

    Serial.print(color[i].clear);

    Serial.print(" RG:");
    Serial.print(color[i].rgRatio);

    Serial.print(" BG:");
    Serial.print(color[i].bgRatio);

    Serial.print(
      color[i].white ? "(WHITE)" : "(GREEN)"
    );
  }

  Serial.println();
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

  stopMotors();

  Wire.begin();
  Wire.setClock(100000);
  Wire.setWireTimeout(30000, false);

  Serial.println("==========================");
  Serial.println("ROBOCUP ATTACKER");
  Serial.println("4 APDS9960 CALIBRATED");
  Serial.println("==========================");

  // MPU6050 initialization
  byte status = mpu.begin();

  if (status != 0) {

    Serial.println("MPU6050 FAILED");

    while (true) {
      stopMotors();
      delay(100);
    }
  }

  Serial.println("Calibrating MPU6050...");
  Serial.println("Keep robot stationary");

  delay(1000);

  mpu.calcOffsets(true, true);
  mpu.update();

  targetHeading = mpu.getAngleZ();

  Serial.println("MPU6050 READY");

  // Initialize all four APDS9960 sensors
  bool allReady = true;

  for (byte i = 0; i < 4; i++) {

    sensorReady[i] = initAPDS(i);

    Serial.print(SENSOR_NAMES[i]);
    Serial.print(": ");

    Serial.println(
      sensorReady[i] ? "READY" : "FAILED"
    );

    if (!sensorReady[i])
      allReady = false;
  }

  if (!allReady) {

    Serial.println("COLOR SENSOR INIT ERROR");

    while (true) {
      stopMotors();
      delay(100);
    }
  }

  // Initial readings with retry
  if (!initializeColorReadings()) {

    Serial.println(
      "ROBOT STOPPED: COLOR INIT FAILED"
    );

    while (true) {
      stopMotors();
      delay(100);
    }
  }

  // IR Seeker initialization
  ballDirection = readBallDirection();

  if (ballDirection < 0) {

    Serial.println("IR SEEKER FAILED");

    while (true) {
      stopMotors();
      delay(100);
    }
  }

  Serial.println("IR SEEKER READY");
  Serial.println("ALL SENSORS READY");
  Serial.println("STARTING IN 3 SECONDS");

  unsigned long startTime = millis();

  while (millis() - startTime < START_DELAY) {
    mpu.update();
    delay(5);
  }

  mpu.update();
  targetHeading = mpu.getAngleZ();

  robotEnabled = true;

  Serial.println("ATTACKER STARTED");
}

// =====================================
// MAIN LOOP
// =====================================

void loop() {

  mpu.update();

  if (!robotEnabled) {
    stopMotors();
    return;
  }

  unsigned long now = millis();

  // PRIORITY 1: Update color sensors
  updateColorSensors();

  // Stop if sensor readings become stale
  if (!colorsHealthy()) {

    stopMotors();

    if (now - lastDebug >= DEBUG_INTERVAL) {
      lastDebug = now;

      Serial.println(
        "WAITING FOR COLOR SENSOR DATA"
      );

      printDebug();
    }

    return;
  }

  // PRIORITY 2: Avoid white boundary
  if (!avoidBoundary()) {

    // PRIORITY 3: Track the ball
    ballDirection = readBallDirection();

    if (ballDirection < 0) {

      stopMotors();

      if (now - lastDebug >= DEBUG_INTERVAL) {
        lastDebug = now;
        Serial.println("IR COMMUNICATION ERROR");
      }

      return;
    }

    attackerLogic(ballDirection);
  }

  // Debug output
  if (now - lastDebug >= DEBUG_INTERVAL) {
    lastDebug = now;
    printDebug();
  }
}
