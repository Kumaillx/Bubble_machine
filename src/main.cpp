#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const int RELAY_PIN = 13;
bool isDeviceOn = false;

void turnDeviceOff() {
    digitalWrite(RELAY_PIN, LOW);
    isDeviceOn = false;
    Serial.println("[BT] Command Executed: OFF");
    SerialBT.println("-> Bubbles are now [OFF]");
}

void turnDeviceOn() {
    digitalWrite(RELAY_PIN, HIGH);
    isDeviceOn = true;
    Serial.println("[BT] Command Executed: ON");
    SerialBT.println("-> Bubbles are now [ON]");
}

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);
    SerialBT.begin("ESP32_Bubble_Machine");
    Serial.println("Bluetooth started. Pair with 'ESP32_Bubble_Machine'");
}

void loop() {
    if (SerialBT.available() > 0) {
        char incomingChar = SerialBT.read();
        incomingChar = toupper(incomingChar);

        if (incomingChar == 'O') {          // ON command
            if (!isDeviceOn) {
                turnDeviceOn();
            }
        } else if (incomingChar == 'F') {    // OFF command
            if (isDeviceOn) {
                turnDeviceOff();
            }
        }
        delay(200);
    }
}