#include <Arduino.h>

// Define the GPIO pin connected to the transistor base circuit
const int RELAY_PIN = 13;

// Timing interval in milliseconds (15 seconds)
const unsigned long INTERVAL = 15000; 

// Variable to track the last time the state was flipped
unsigned long previousMillis = 0;

// Track the current operational state
bool isDeviceOn = false;

void turnDeviceOff() {
    digitalWrite(RELAY_PIN, LOW); // Turns transistor off -> Relay drops -> OFF
    isDeviceOn = false;
    Serial.print("[TIMER STATUS] ");
    Serial.print(millis() / 1000);
    Serial.println("s - Turning bubbles OFF. Waiting 15 seconds...");
}

void turnDeviceOn() {
    digitalWrite(RELAY_PIN, HIGH); // Turns transistor on -> Grounds relay input -> ON
    isDeviceOn = true;
    Serial.print("[TIMER STATUS] ");
    Serial.print(millis() / 1000);
    Serial.println("s - Turning bubbles ON! Running for 15 seconds...");
}

void setup() {
    // Initialize serial communication for monitoring the cycle
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port connection
    }

    pinMode(RELAY_PIN, OUTPUT);
    
    Serial.println("=== Automated 15-Second Bubble Machine Loop ===");
    
    // Start the cycle with the device ON
    turnDeviceOn();
    previousMillis = millis();
}

void loop() {
    // Get the current running time
    unsigned long currentMillis = millis();

    // Check if 15 seconds have passed since the last switch
    if (currentMillis - previousMillis >= INTERVAL) {
        // Save the timestamp for the next switch window
        previousMillis = currentMillis;

        // Toggle between ON and OFF states
        if (isDeviceOn) {
            turnDeviceOff();
        } else {
            turnDeviceOn();
        }
    }
}