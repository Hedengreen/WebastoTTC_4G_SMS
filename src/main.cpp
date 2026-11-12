
#include <Arduino.h>
#include "project_config.h"

HardwareSerial SerialAT(1);

void sendAT(const String& command, uint32_t timeout = 3000) {
    Serial.println("\n>> " + command);

    SerialAT.println(command);

    uint32_t start = millis();

    while (millis() - start < timeout) {
        while (SerialAT.available()) {
            Serial.write(SerialAT.read());
        }
        delay(1);
    }
}

bool waitForModem(uint32_t timeout = 30000) {
    uint32_t start = millis();

    while (millis() - start < timeout) {
        SerialAT.println("AT");

        String response;
        uint32_t attempt = millis();

        while (millis() - attempt < 1000) {
            while (SerialAT.available()) {
                response += (char)SerialAT.read();
            }
            delay(1);
        }

        if (response.indexOf("OK") >= 0) {
            return true;
        }

        Serial.println("Waiting for modem...");
    }

    return false;
}

void startModem() {
    Serial.println("\nStarting modem...");

    // Enable modem power supply
    pinMode(MODEM_POWERON_PIN, OUTPUT);
    digitalWrite(MODEM_POWERON_PIN, HIGH);

    // Keep modem awake
    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, LOW);

    // Start modem UART
    SerialAT.begin(
        MODEM_BAUDRATE,
        SERIAL_8N1,
        MODEM_RX_PIN,
        MODEM_TX_PIN
    );

    delay(500);

    // Power-on key pulse
    pinMode(MODEM_PWRKEY_PIN, OUTPUT);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(100);

    digitalWrite(MODEM_PWRKEY_PIN, HIGH);
    delay(1000);

    digitalWrite(MODEM_PWRKEY_PIN, LOW);

    if (!waitForModem()) {
        Serial.println("ERROR: Modem did not respond!");
        return;
    }

    Serial.println("Modem is responding!");

    sendAT("ATE0");                 // Disable command echo
    sendAT("AT+CPIN?");             // SIM status
    sendAT("AT+SIMCOMATI");         // Modem identification
    sendAT("AT+CSQ");               // Signal quality
    sendAT("AT+CEREG?");            // LTE network registration

    // SMS text mode
    sendAT("AT+CMGF=1");

    // GSM character set
    sendAT("AT+CSCS=\"GSM\"");

    // Notify ESP32 when a new SMS arrives
    sendAT("AT+CNMI=2,1,0,0,0");

    Serial.println("\nModem initialization finished.");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n==============================");
    Serial.println("MULTIVAN WEBSTO CONTROLLER");
    Serial.println("==============================");

    startModem();
}

void loop() {
    // Temporary AT-command passthrough.
    // Allows us to test the modem from Serial Monitor.

    while (Serial.available()) {
        SerialAT.write(Serial.read());
    }

    while (SerialAT.available()) {
        Serial.write(SerialAT.read());
    }

    delay(1);
}
