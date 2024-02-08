#include <Arduino.h>
#include "vectoring.hpp"
#include "calibration.hpp"

#define THROTTLE_PIN A0
#define STEERING_PIN A1
#define CALIBRATION_PIN 2
#define CALIBRATION_INDICATOR_PIN 13
#define SAFETY_CALIBRATION_PIN 4
#define CLUTCH_PIN 3

Vectoring a = Vectoring("steering-angles.csv");
Calibration c = Calibration(STEERING_PIN, CALIBRATION_PIN, CALIBRATION_INDICATOR_PIN, a);
Calibration *Calibration::instance = &c;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello, World! from setup()");
  attachInterrupt(digitalPinToInterrupt(c.get_buttonPin()), Calibration::calibrate, RISING); // TODO safety thing for accidental calibration
}

void loop()
{
  int throttle = analogRead(THROTTLE_PIN);
  int steering = analogRead(STEERING_PIN);

  a.update_throttle(map(throttle, 0, 1023, 0, 100));
  a.update_steer_travel(a.convert_to_degrees(steering));
  a.calculate_torque();
  Serial.print(a.print());

  delay(1000);
}
