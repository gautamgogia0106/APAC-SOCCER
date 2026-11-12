
#include <Arduino.h>

// ======================================
// L298N MOTOR PINS
// ======================================

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

// ======================================
// MOTOR FORWARD DIRECTIONS
// ======================================
// Change 1 to -1 if a motor rotates
// opposite to its required direction.

const int FR_FORWARD = 1;
const int BR_FORWARD = 1;
const int BL_FORWARD = 1;
const int FL_FORWARD = 1;

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
// FORWARD
// ======================================

void forward() {

  motor(FR_IN1, FR_IN2, FR_FORWARD);
  motor(BR_IN1, BR_IN2, BR_FORWARD);

  motor(BL_IN1, BL_IN2, BL_FORWARD);
  motor(FL_IN1, FL_IN2, FL_FORWARD);
}

// ======================================
// BACKWARD
// ======================================

void backward() {

  motor(FR_IN1, FR_IN2, -FR_FORWARD);
  motor(BR_IN1, BR_IN2, -BR_FORWARD);

  motor(BL_IN1, BL_IN2, -BL_FORWARD);
  motor(FL_IN1, FL_IN2, -FL_FORWARD);
}

// ======================================
// STRAFE LEFT
// ======================================

void strafeLeft() {

  motor(FR_IN1, FR_IN2, FR_FORWARD);
  motor(BR_IN1, BR_IN2, -BR_FORWARD);

  motor(BL_IN1, BL_IN2, BL_FORWARD);
  motor(FL_IN1, FL_IN2, -FL_FORWARD);
}

// ======================================
// STRAFE RIGHT
// ======================================

void strafeRight() {

  motor(FR_IN1, FR_IN2, -FR_FORWARD);
  motor(BR_IN1, BR_IN2, BR_FORWARD);

  motor(BL_IN1, BL_IN2, -BL_FORWARD);
  motor(FL_IN1, FL_IN2, FL_FORWARD);
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

  Serial.println("SOCCER ROBOT MOVEMENT TEST");

  delay(2000);
}

// ======================================
// MAIN LOOP
// ======================================

void loop() {

  Serial.println("FORWARD");
  forward();
  delay(3000);

  stopAll();
  delay(1500);

  Serial.println("BACKWARD");
  backward();
  delay(3000);

  stopAll();
  delay(1500);

  Serial.println("STRAFE LEFT");
  strafeLeft();
  delay(3000);

  stopAll();
  delay(1500);

  Serial.println("STRAFE RIGHT");
  strafeRight();
  delay(3000);

  stopAll();
  delay(3000);
}
