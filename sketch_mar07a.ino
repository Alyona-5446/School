#include <Servo.h>
#include <SoftwareSerial.h>
#include <Stepper.h>

#define FIRE 1
#define PULL 2
#define ROTATE 3
#define RELOAD 4
#define MODIFY_STRENGTH 5
#define ELSE 6

#define SERIAL_SPEED 9600
#define S_SERIAL_SPEED 9600

#define PULLING_SPEED 135
#define PULLING_STOP 90
#define ROTATE_ST_SPEED 50
#define STRENGTH_ST_SPEED 50

#define PULLING_TIME 1500
#define RELOAD_TIME 500
#define SERVO_TIME 250
#define STEP_TIME 50

#define RELOAD_OPEN 0
#define RELOAD_CLOSED 90
#define LOCK_OPEN 0
#define LOCK_CLOSED 90

#define MAX_ANGLE 540

#define TORSION_STEP 10

#define STEP_360 360

#define ROTATE_ST_A1_PIN 8 //A0
#define ROTATE_ST_A2_PIN 9 //A1
#define ROTATE_ST_B1_PIN 10 //A2
#define ROTATE_ST_B2_PIN 11 //A3

#define STRENGTH_ST_A1_PIN 2
#define STRENGTH_ST_A2_PIN 3
#define STRENGTH_ST_B1_PIN 4
#define STRENGTH_ST_B2_PIN A4

#define RELOAD_S_PIN 8

#define LOCK_S_PIN 9

#define PULL_S_PIN 10

#define BT_RX_PIN 5
#define BT_TX_PIN 6

SoftwareSerial SoftSerial(BT_RX_PIN, BT_TX_PIN);
Servo reload_servo, lock_servo, pull_servo;

Stepper rotate_stepper(STEP_360, ROTATE_ST_A1_PIN, ROTATE_ST_A2_PIN,
ROTATE_ST_B1_PIN, ROTATE_ST_B2_PIN),
strength_stepper(STEP_360, STRENGTH_ST_A1_PIN, STRENGTH_ST_A2_PIN,
STRENGTH_ST_B1_PIN, STRENGTH_ST_B2_PIN);

int cur_pos = 0;

void reload() {
  reload_servo.attach(RELOAD_S_PIN);
  reload_servo.write(RELOAD_OPEN);
  delay(RELOAD_TIME);
  reload_servo.write(RELOAD_CLOSED);
  delay(SERVO_TIME);
  reload_servo.detach();
}

void pull() {
  pull_servo.attach(PULL_S_PIN);
  pull_servo.write(PULLING_SPEED);
  delay(PULLING_TIME);
  pull_servo.write(PULLING_STOP);
  lock_servo.attach(LOCK_S_PIN);
  lock_servo.write(LOCK_CLOSED);
  delay(SERVO_TIME);
  lock_servo.detach();
  pull_servo.write(-PULLING_SPEED);
  delay(PULLING_TIME);
  pull_servo.write(PULLING_STOP);
  pull_servo.detach();
}

void fire() {
  lock_servo.attach(LOCK_S_PIN);
  lock_servo.write(LOCK_OPEN);
  delay(SERVO_TIME);
  lock_servo.detach();
}

void rotate(int delta) {
  int new_pos = (cur_pos + (delta % MAX_ANGLE) + MAX_ANGLE) % MAX_ANGLE;
  rotate_stepper.step(new_pos - cur_pos);
  delay(abs(new_pos - cur_pos) * STEP_TIME);
  cur_pos = new_pos;
}

void modify_strength(int steps) {
  strength_stepper.step(TORSION_STEP * steps);
  delay(abs(steps) * STEP_TIME);
}

void setup() {
  SoftSerial.begin(S_SERIAL_SPEED);
  Serial.begin(SERIAL_SPEED);
  rotate_stepper.setSpeed(ROTATE_ST_SPEED);
  strength_stepper.setSpeed(STRENGTH_ST_SPEED);
}

void loop() {
  rotate_stepper.step(360);
  delay(10 * STEP_TIME);
  rotate_stepper.step(-360);
  delay(10 * STEP_TIME);
  if (SoftSerial.available()) {
    byte incoming = SoftSerial.read();
    if (incoming) {
      switch(incoming) {
        case FIRE: {
          Serial.print("Fire\n");
          fire();
          SoftSerial.write((byte)FIRE);
          break;
        }
        case PULL: {
          Serial.print("Pull\n");
          pull();
          SoftSerial.write((byte)PULL);
          break;
        }
        case ROTATE: {
          Serial.print("Rotate\n");
          byte byte1 = SoftSerial.read();
          Serial.print(byte1);
          Serial.print('\n');
          byte byte2 = SoftSerial.read();
          Serial.print(byte2);
          Serial.print('\n');
          int angle = byte1 | ((int)byte2 << 8);
          Serial.print(angle);
          Serial.print('\n');
          for (int i = 15; i >= 0; --i) {
            Serial.print((angle & (1 << i)) >> i);
          }
          Serial.print('\n');
          SoftSerial.write((byte)ROTATE);
          break;
        }
        case RELOAD: {
          Serial.print("Reload\n");
          reload();
          SoftSerial.write((byte)RELOAD);
          break;
        }
        case MODIFY_STRENGTH: {
          Serial.print("Modify strength\n");
          int steps = SoftSerial.read() | ((int)SoftSerial.read() << 8);
          Serial.print(steps);
          Serial.print('\n');
          modify_strength(steps);
          SoftSerial.write((byte)MODIFY_STRENGTH);
          break;
        }
        default: {
          Serial.print("Else\n");
          SoftSerial.write((byte)ELSE);
          break;
        }
      }
    }
  }
}
