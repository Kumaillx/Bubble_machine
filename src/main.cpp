/*
 * ============================================================
 *  Bubble Machine – ESP32 + Relay via Bluetooth Serial
 * ============================================================
 *
 *  Hardware:
 *    - ESP32 DOIT DevKit v1
 *    - Relay module IN pin  → GPIO 26  (active-LOW or active-HIGH, see below)
 *    - Relay module VCC     → 5 V or 3.3 V (check your module specs)
 *    - Relay module GND     → GND
 *
 *  Bluetooth:
 *    - Classic BT SPP (Serial Port Profile)
 *    - Device name: "BubbleMachine"
 *    - Pair from phone / PC, then open a serial terminal at any baud
 *
 *  Supported commands (case-insensitive, newline-terminated):
 *    ON              – turn relay ON  (motor runs, bubbles!)
 *    OFF             – turn relay OFF (motor stops)
 *    PULSE <ms>      – turn ON for <ms> milliseconds then OFF
 *                      e.g.  PULSE 3000  → run for 3 seconds
 *    STATUS          – reply with current relay state
 *    HELP            – list commands
 *
 * ============================================================
 */

#include <Arduino.h>
#include <BluetoothSerial.h>

// ── Configuration ────────────────────────────────────────────
#define RELAY_PIN       26      // GPIO pin connected to relay IN
#define RELAY_ON_LEVEL  LOW     // Set LOW for active-LOW relay modules
                                // Set HIGH for active-HIGH relay modules
#define RELAY_OFF_LEVEL HIGH    // Opposite of RELAY_ON_LEVEL
#define BT_DEVICE_NAME  "BubbleMachine"
// ─────────────────────────────────────────────────────────────

BluetoothSerial SerialBT;

// ── State ────────────────────────────────────────────────────
bool   relayOn        = false;
bool   pulseActive    = false;
unsigned long pulseEndMs = 0;

// ── Helpers ──────────────────────────────────────────────────

/** Set the physical relay and update state flag. */
void setRelay(bool on) {
    relayOn = on;
    digitalWrite(RELAY_PIN, on ? RELAY_ON_LEVEL : RELAY_OFF_LEVEL);
    String msg = on ? ">> Relay ON  – Bubbles GO! 🫧\n"
                    : ">> Relay OFF – Bubbles stopped.\n";
    Serial.print(msg);
    SerialBT.print(msg);
}

/** Send a message to both Serial monitor and BT client. */
void sendMsg(const String &msg) {
    Serial.print(msg);
    SerialBT.print(msg);
}

/** Parse and dispatch a command string. */
void handleCommand(String cmd) {
    cmd.trim();
    String upper = cmd;
    upper.toUpperCase();

    if (upper == "ON") {
        pulseActive = false;
        setRelay(true);

    } else if (upper == "OFF") {
        pulseActive = false;
        setRelay(false);

    } else if (upper.startsWith("PULSE ")) {
        // Extract duration in ms
        String durStr = cmd.substring(6);
        durStr.trim();
        long dur = durStr.toInt();
        if (dur <= 0) {
            sendMsg(">> ERROR: invalid duration. Usage: PULSE <milliseconds>\n");
            return;
        }
        pulseActive  = true;
        pulseEndMs   = millis() + (unsigned long)dur;
        setRelay(true);
        String info = ">> PULSE for " + String(dur) + " ms\n";
        sendMsg(info);

    } else if (upper == "STATUS") {
        String status = ">> STATUS: Relay is ";
        status += relayOn ? "ON" : "OFF";
        if (pulseActive) {
            unsigned long remaining = (pulseEndMs > millis())
                                      ? pulseEndMs - millis() : 0;
            status += " (PULSE – " + String(remaining) + " ms remaining)";
        }
        status += "\n";
        sendMsg(status);

    } else if (upper == "HELP") {
        sendMsg(
            ">> --- Bubble Machine Commands ---\n"
            ">>  ON            Turn relay ON\n"
            ">>  OFF           Turn relay OFF\n"
            ">>  PULSE <ms>    Run for <ms> milliseconds\n"
            ">>  STATUS        Show current state\n"
            ">>  HELP          Show this help\n"
            "----------------------------------\n"
        );

    } else if (upper.length() > 0) {
        sendMsg(">> Unknown command: \"" + cmd + "\"\n"
                ">> Type HELP for available commands.\n");
    }
}

// ── Incoming command buffer ───────────────────────────────────
String btBuffer = "";

// ── setup() ──────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    // Configure relay pin
    pinMode(RELAY_PIN, OUTPUT);
    setRelay(false);   // ensure relay is OFF at startup

    // Start Bluetooth
    if (!SerialBT.begin(BT_DEVICE_NAME)) {
        Serial.println("[ERROR] Bluetooth init failed! Halting.");
        while (true) { delay(1000); }
    }

    Serial.println("=================================");
    Serial.println("  Bubble Machine – Ready!");
    Serial.println("  BT Name : " BT_DEVICE_NAME);
    Serial.println("  Relay   : GPIO " + String(RELAY_PIN));
    Serial.println("  Waiting for Bluetooth connection…");
    Serial.println("=================================");
}

// ── loop() ───────────────────────────────────────────────────
void loop() {
    // ── Handle pulse timeout ──────────────────────────────────
    if (pulseActive && millis() >= pulseEndMs) {
        pulseActive = false;
        setRelay(false);
        sendMsg(">> PULSE complete – relay OFF.\n");
    }

    // ── Read from Bluetooth ───────────────────────────────────
    while (SerialBT.available()) {
        char c = (char)SerialBT.read();
        if (c == '\n' || c == '\r') {
            if (btBuffer.length() > 0) {
                handleCommand(btBuffer);
                btBuffer = "";
            }
        } else {
            btBuffer += c;
        }
    }

    // ── Read from USB Serial (for local testing) ──────────────
    while (Serial.available()) {
        char c = (char)Serial.read();
        if (c == '\n' || c == '\r') {
            if (btBuffer.length() > 0) {
                handleCommand(btBuffer);
                btBuffer = "";
            }
        } else {
            btBuffer += c;
        }
    }
}