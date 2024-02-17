#pragma once
#include <Arduino.h>
#include "vectoring.hpp"

enum calibrationState
{
    LEFT,
    MID,
    RIGHT
};

// TODO saving and loading calibration
class Calibration
{
private:
    int analogPinIn;
    int button;
    int ledOut;
    calibrationState state = LEFT;
    Vectoring &vec;

public:
    static Calibration *instance;

    Calibration(int analogPin, int buttonPin, int outputPin, Vectoring &vec) : analogPinIn(analogPin), ledOut(outputPin), vec(vec), button(buttonPin)
    {
        instance = this;
        pinMode(buttonPin, INPUT_PULLUP);
    }

    static void calibrate()
    {
        if (instance != nullptr)
            instance->buttonPressed();

        else
        {
            while (true)
            {
                Serial.println("Instance of callibration is null");
                digitalWrite(instance->ledOut, HIGH);
                delay(1000);
            }
        }
    }

    void buttonPressed()
    {

        // debounce button
        static unsigned long last_interrupt_time = 0;
        unsigned long interrupt_time = millis();
        if (interrupt_time - last_interrupt_time < 50)
            return;

        // calculate average
        volatile int total = 0;
        volatile int sampleCount = 0;
        while (sampleCount < 50)
        {
            total += analogRead(analogPinIn);
            sampleCount++;
        }

        int average = total / 50;
        switch (state)
        {
        case LEFT:
            vec.wheel.left = average;
            analogWrite(ledOut, 255);
            break;
        case MID:
            vec.wheel.middle = average;
            analogWrite(ledOut, 128);
            break;
        case RIGHT:
            vec.wheel.right = average;
            analogWrite(ledOut, 0);
            break;
        default:
            break;
        }
        state = static_cast<calibrationState>((state + 1) % 3);
        last_interrupt_time = interrupt_time;
    }

    int get_analogPinIn() const
    {
        return analogPinIn;
    }
    int get_buttonPin() const
    {
        return button;
    }
    int get_ledOut() const
    {
        return ledOut;
    }
};