#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// Define the GPIO pin connected to your BC547 transistor base circuit
const int RELAY_PIN = 13;
bool isDeviceOn = false;

void turnDeviceOff() {
    digitalWrite(RELAY_PIN, LOW); // Transistor OFF -> Relay drops -> OFF
    isDeviceOn = false;
    Serial.println("[BT] Command Executed: OFF");
    SerialBT.println("-> Bubbles are now [OFF]");
}

void turnDeviceOn() {
    digitalWrite(RELAY_PIN, HIGH); // Transistor ON -> Relay triggers -> ON
    isDeviceOn = true;
    Serial.println("[BT] Command Executed: ON");
    SerialBT.println("-> Bubbles are now [ON]");
}

void setup() {
    Serial.begin(115200);
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW); // Ensure it starts safely OFF

    // Broadcast name for pairing
    SerialBT.begin("ESP32_Bubble_Machine"); 
    Serial.println("Bluetooth started. Pair with 'ESP32_Bubble_Machine'");
}

void loop() {
    // Process characters immediately as they arrive over the air
    if (SerialBT.available() > 0) {
        char incomingChar = SerialBT.read();
        
        // Convert to uppercase so 'y' and 'Y' both pass
        incomingChar = toupper(incomingChar);

        if (incomingChar == 'Y') {
            if (!isDeviceOn) {
                turnDeviceOn();
            } else {
                turnDeviceOff();
            }
            delay(200); // Small debounce protection
        }
    }
}