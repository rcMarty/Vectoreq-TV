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
Calibration steering = Calibration(STEERING_PIN, CALIBRATION_PIN, CALIBRATION_INDICATOR_PIN, a);
Calibration *Calibration::instance = &steering;

void setup()
{
  Serial.begin(115200);
  Serial.println("Hello, World! from setup()");
  attachInterrupt(digitalPinToInterrupt(steering.get_buttonPin()), Calibration::calibrate, RISING); // TODO safety thing for accidental calibration
}

void loop()
{
  if (digitalRead(CLUTCH_PIN) == HIGH)
  {
    a.update_throttle(0);
    a.update_steer_travel(0);
  }
  else
  {
    int throttle = analogRead(THROTTLE_PIN);
    int steering = analogRead(STEERING_PIN);

    a.update_throttle(map(throttle, 0, 1023, 0, 100));
    a.update_steer_travel(a.convert_to_degrees(steering));
    a.calculate_torque();
    Serial.print(a.print());
  }

  delay(1000);
}
