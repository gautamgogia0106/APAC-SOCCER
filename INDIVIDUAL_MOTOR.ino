
#include <Arduino.h>

// ======================================
// L298N MOTOR PINS
// ======================================

// RIGHT MOTOR DRIVER

// FRONT RIGHT - IN1, IN2
#define FR_IN1 3
#define FR_IN2 2

// BACK RIGHT - IN3, IN4
#define BR_IN1 4
#define BR_IN2 5

// LEFT MOTOR DRIVER

// BACK LEFT - IN1, IN2
#define BL_IN1 9
#define BL_IN2 8

// FRONT LEFT - IN3, IN4
#define FL_IN1 11
#define FL_IN2 10

// ======================================
// MOTOR CONTROL
// ======================================

void motor(int pin1, int pin2, int direction) {

  if (direction == 1) {
    digitalWrite(pin1, HIGH);
    digitalWrite(pin2, LOW);
  }
  else if (direction == -1) {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, HIGH);
  }
  else {
    digitalWrite(pin1, LOW);
    digitalWrite(pin2, LOW);
  }
}

// ======================================
// STOP ALL MOTORS
// ======================================

void stopAll() {
  motor(FR_IN1, FR_IN2, 0);
  motor(BR_IN1, BR_IN2, 0);
  motor(BL_IN1, BL_IN2, 0);
  motor(FL_IN1, FL_IN2, 0);
}

// ======================================
// INDIVIDUAL MOTOR TEST
// ======================================

void testMotor(const char* name, int pin1, int pin2) {

  stopAll();

  Serial.println("---------------------");
  Serial.print("TESTING: ");
  Serial.println(name);

  // DIRECTION 1
  Serial.println("DIRECTION 1");
  motor(pin1, pin2, 1);
  delay(3000);

  motor(pin1, pin2, 0);
  delay(1500);

  // DIRECTION -1
  Serial.println("DIRECTION -1");
  motor(pin1, pin2, -1);
  delay(3000);

  motor(pin1, pin2, 0);
  delay(1500);

  Serial.println("TEST COMPLETED");
}

// ======================================
// SETUP
// ======================================

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

  Serial.println("L298N MOTOR TEST");
  delay(2000);
}

// ======================================
// MAIN LOOP
// ======================================

void loop() {

  testMotor("FRONT RIGHT", FR_IN1, FR_IN2);

  testMotor("BACK RIGHT", BR_IN1, BR_IN2);

  testMotor("BACK LEFT", BL_IN1, BL_IN2);

  testMotor("FRONT LEFT", FL_IN1, FL_IN2);

  stopAll();

  Serial.println("ALL MOTOR TESTS COMPLETE");
  delay(5000);
}
