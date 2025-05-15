#include <Arduino.h>

#include "feeder.h"

#define PROBE_PIN D7
#define MOTOR_PIN D6

Feeder::Feeder()
{
}

void Feeder::init()
{
    pinMode(MOTOR_PIN, OUTPUT);
    pinMode(PROBE_PIN, INPUT_PULLUP);
    digitalWrite(MOTOR_PIN, LOW); // Motor off
}

void Feeder::feed()
{
    digitalWrite(MOTOR_PIN, HIGH); // Turn motor on

    // Debounce variables
    const int debounceDelay = 50;       // Debounce time in milliseconds
    int probeState = LOW;               // Current state of the probe pin
    int lastProbeState = LOW;           // Previous state of the probe pin
    unsigned long lastDebounceTime = 0; // Last time the probe pin was toggled

    // State tracking variables
    bool sawHigh = false;
    bool sawLow = false;

    // Wait for the probe pin to go HIGH->LOW->HIGH
    while (!(sawHigh && sawLow && probeState == HIGH))
    {
        // Read the current state of the probe pin
        int reading = digitalRead(PROBE_PIN);

        // If the reading changed, reset the debounce timer
        if (reading != lastProbeState)
        {
            lastDebounceTime = millis();
        }

        // If the reading has been stable for longer than the debounce delay
        if ((millis() - lastDebounceTime) > debounceDelay)
        {
            // If the debounced state has changed
            if (reading != probeState)
            {
                probeState = reading;

                // Update our state tracking
                if (probeState == HIGH)
                {
                    if (!sawHigh)
                    {
                        sawHigh = true; // First HIGH
                    }
                    else if (sawLow)
                    {
                        // We've seen HIGH->LOW->HIGH, break out of the loop
                        break;
                    }
                }
                else if (probeState == LOW)
                {
                    if (sawHigh)
                    {
                        sawLow = true; // HIGH->LOW
                    }
                }
            }
        }

        lastProbeState = reading;

        // Let other processes run
        yield();
    }

    // After detecting the cycle, turn off the motor
    digitalWrite(MOTOR_PIN, LOW);
}