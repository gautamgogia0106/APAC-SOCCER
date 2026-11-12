
#include <Arduino.h>
#include <Wire.h>

// =====================================
// ROBOCUP COLOR THRESHOLD TEST
// ARDUINO MEGA + TCA9548A + APDS9960
// =====================================

#define TCA_ADDR  0x70
#define APDS_ADDR 0x39

#define FRONT 0
#define BACK  1
#define LEFT  2
#define RIGHT 3

// TCA CHANNEL MAPPING
const byte CHANNELS[4] = {
  6,  // FRONT
  1,  // BACK
  2,  // LEFT
  0,   // RIGHT
};

const char* SENSOR_NAMES[4] = {
  "FRONT", "BACK", "LEFT", "RIGHT"
};

// =====================================
// WHITE RGB THRESHOLDS
// Provisional values from earlier tests
// =====================================

// RED / GREEN * 1000
int WHITE_RG_MIN[4] = {
  625,  // FRONT
  735,  // BACK
  690,  // LEFT
  800   // RIGHT
};

// BLUE / GREEN * 1000
int WHITE_BG_MIN[4] = {
  620,  // FRONT
  650,  // BACK
  590,  // LEFT
  620   // RIGHT
};

// =====================================
// APDS9960 REGISTERS
// =====================================

#define ENABLE_REG  0x80
#define ATIME_REG   0x81
#define CONTROL_REG 0x8F
#define ID_REG      0x92
#define STATUS_REG  0x93

#define CDATAL_REG  0x94
#define RDATAL_REG  0x96
#define GDATAL_REG  0x98
#define BDATAL_REG  0x9A

bool sensorReady[4] = {
  false, false, false, false
};

// =====================================
// SELECT TCA CHANNEL
// =====================================

bool selectChannel(byte channel) {

  Wire.beginTransmission(TCA_ADDR);
  Wire.write((byte)0);

  if (Wire.endTransmission() != 0)
    return false;

  delay(2);

  Wire.beginTransmission(TCA_ADDR);
  Wire.write((byte)(1 << channel));

  if (Wire.endTransmission() != 0)
    return false;

  delay(3);
  return true;
}

// =====================================
// READ REGISTER
// =====================================

int readReg(byte reg) {

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

// =====================================
// WRITE REGISTER
// =====================================

bool writeReg(byte reg, byte value) {

  Wire.beginTransmission(APDS_ADDR);
  Wire.write(reg);
  Wire.write(value);

  return Wire.endTransmission() == 0;
}

// =====================================
// READ 16-BIT VALUE
// =====================================

bool read16(byte reg, uint16_t &value) {

  int low = readReg(reg);
  int high = readReg(reg + 1);

  if (low < 0 || high < 0)
    return false;

  value = (uint16_t)low |
          ((uint16_t)high << 8);

  return true;
}

// =====================================
// INITIALIZE SENSOR
// =====================================

bool initSensor(byte index) {

  if (!selectChannel(CHANNELS[index]))
    return false;

  int id = readReg(ID_REG);

  if (id != 0x9E) {
    Serial.print(SENSOR_NAMES[index]);
    Serial.println(" ID ERROR");
    return false;
  }

  bool ok = true;

  ok &= writeReg(ENABLE_REG, 0x00);
  delay(10);

  ok &= writeReg(ATIME_REG, 0xDB);
  ok &= writeReg(CONTROL_REG, 0x02);
  ok &= writeReg(ENABLE_REG, 0x03);

  delay(50);

  return ok;
}

// =====================================
// WHITE DETECTION
// =====================================

bool detectWhite(byte index,
                 uint16_t r,
                 uint16_t g,
                 uint16_t b,
                 int &rg,
                 int &bg) {

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
// READ SENSOR
// =====================================

void readSensor(byte index) {

  Serial.print(SENSOR_NAMES[index]);
  Serial.print(" | ");

  if (!sensorReady[index]) {
    Serial.println("NOT INITIALIZED");
    return;
  }

  if (!selectChannel(CHANNELS[index])) {
    Serial.println("TCA ERROR");
    return;
  }

  int status = readReg(STATUS_REG);

  if (status < 0) {
    Serial.println("I2C ERROR");
    return;
  }

  if (!(status & 0x01)) {
    Serial.println("DATA NOT READY");
    return;
  }

  uint16_t c, r, g, b;

  bool ok =
    read16(CDATAL_REG, c) &&
    read16(RDATAL_REG, r) &&
    read16(GDATAL_REG, g) &&
    read16(BDATAL_REG, b);

  if (!ok) {
    Serial.println("READ ERROR");
    return;
  }

  int rg, bg;

  bool white = detectWhite(
    index, r, g, b, rg, bg
  );

  Serial.print("C:");
  Serial.print(c);

  Serial.print(" R:");
  Serial.print(r);

  Serial.print(" G:");
  Serial.print(g);

  Serial.print(" B:");
  Serial.print(b);

  Serial.print(" | RG:");
  Serial.print(rg);

  Serial.print(" BG:");
  Serial.print(bg);

  Serial.print(" | ");

  if (white)
    Serial.println("WHITE");
  else
    Serial.println("GREEN");
}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);

  Wire.begin();
  Wire.setClock(100000);
  Wire.setWireTimeout(30000, false);

  delay(500);

  Serial.println();
  Serial.println("==========================");
  Serial.println("ROBOCUP COLOR CALIBRATION");
  Serial.println("==========================");

  for (byte i = 0; i < 4; i++) {

    sensorReady[i] = initSensor(i);

    Serial.print(SENSOR_NAMES[i]);

    if (sensorReady[i])
      Serial.println(" READY");
    else
      Serial.println(" FAILED");

    delay(100);
  }

  Serial.println("==========================");
}

// =====================================
// LOOP
// =====================================

void loop() {

  Serial.println();
  Serial.println("------ SENSOR VALUES ------");

  for (byte i = 0; i < 4; i++) {
    readSensor(i);
    delay(30);
  }

  Serial.println("---------------------------");

  delay(300);
}
