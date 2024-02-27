#include <Arduino.h>
#include <ezButton.h>
#include "vectoring.hpp"
#include "calibration.hpp"
#include "include.h"

#define THROTTLE_PIN TPS
#define STEERING_PIN STEER
#define CALIBRATION_PIN D3
#define CALIBRATION_INDICATOR_PIN D1
#define SAFETY_CALIBRATION_PIN D0
#define CLUTCH_PIN CLUTCH

ezButton buttD0(D0);
Vectoring *a;
// Calibration c = Calibration(STEERING_PIN, CALIBRATION_PIN, CALIBRATION_INDICATOR_PIN, a);
// Calibration *Calibration::instance = &c;

void setup()
{
  Serial.begin(115200);

  Serial.println("Hello, World! from setup()");
  // attachInterrupt(digitalPinToInterrupt(c.get_buttonPin()), Calibration::calibrate, RISING); // TODO safety thing for accidental calibration

  pinMode(THROTTLE_PIN, INPUT);
  pinMode(STEERING_PIN, INPUT);
  pinMode(CLUTCH_PIN, INPUT);
  a = new Vectoring("");
}

void loop()
{

  // setup ezButton
  buttD0.loop();

  int throttle = analogRead(THROTTLE_PIN);
  int steering = analogRead(STEERING_PIN);

  if (buttD0.isPressed())
  {
    Serial.println("Next modifier!");
    a->next_modifier();
  }

  a->update_throttle(map(throttle, 0, 4098, 0, 100));
  a->update_steer_travel(a->convert_to_degrees(steering));
  a->calculate_torque();
  Serial.print(a->print());

  delay(1000);
}
