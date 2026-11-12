
#include <Arduino.h>

#include "project_config.h"
#include "secrets.h"
#include "modem.h"
#include "sms_handler.h"

// ========================================
// MODEM
// ========================================

HardwareSerial SerialAT(1);

bool modemReady = false;

unsigned long lastSmsCheck = 0;


// ========================================
// STARTA MODEMET
// ========================================

bool startModem() {
    Serial.println("\nStarting modem...");

    // Aktivera modemets strömförsörjning
    pinMode(MODEM_POWERON_PIN, OUTPUT);
    digitalWrite(MODEM_POWERON_PIN, HIGH);

    // Håll modemet vaket
    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, LOW);

    // Starta modemets UART
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

    // Kontrollera att modemet svarar
    unsigned long start = millis();

    while (millis() - start < 30000) {
        while (SerialAT.available()) {
            SerialAT.read();
        }

        SerialAT.println("AT");

        String response;
        unsigned long attempt = millis();

        while (millis() - attempt < 1000) {
            while (SerialAT.available()) {
                response += (char)SerialAT.read();
            }

            if (response.indexOf("OK") >= 0) {
                Serial.println("Modem is responding!");
                return true;
            }

            delay(1);
        }

        Serial.println("Waiting for modem...");
    }

    Serial.println("ERROR: Modem did not respond!");

    return false;
}


// ========================================
// BEHANDLA MOTTAGET SMS
// ========================================

void processIncomingSms() {
    SmsMessage sms;

    // Försök läsa ett oläst SMS
    if (!modemReadNextSms(sms)) {
        return;
    }

    Serial.println("\nNew SMS received!");

    Serial.println("Sender: " + sms.sender);
    Serial.println("Message: " + sms.text);

    // Kontrollera avsändarens behörighet
    if (!isAuthorizedNumber(sms.sender)) {
        Serial.println("Unauthorized sender!");

        // Radera SMS så att det inte behandlas igen
        modemDeleteSms(sms.index);

        return;
    }

    Serial.println("Authorized sender.");

    // Tolka SMS-kommandot
    SmsCommand command = parseSmsCommand(sms.text);

    // Skapa svar
    String response = getCommandResponse(command);

    Serial.println("Sending response...");

    // Skicka svar till avsändaren
    bool sent = modemSendSms(sms.sender, response);

    if (sent) {
        Serial.println("Response SMS sent.");
    } else {
        Serial.println("Failed to send response SMS.");
    }

    // Radera SMS efter hantering
    if (modemDeleteSms(sms.index)) {
        Serial.println("SMS deleted.");
    } else {
        Serial.println("Could not delete SMS.");
    }
}


// ========================================
// SETUP
// ========================================

void setup() {
    Serial.begin(115200);

    delay(1000);

    Serial.println("\n==============================");
    Serial.println("MULTIVAN WEBSTO CONTROLLER");
    Serial.println("==============================");

    // Starta modemet
    if (!startModem()) {
        Serial.println("Modem startup failed.");
        return;
    }

    // Konfigurera SMS-funktionen
    modemReady = modemBegin();

    if (modemReady) {
        Serial.println("SMS system ready!");
    } else {
        Serial.println("SMS initialization failed!");
    }
}


// ========================================
// LOOP
// ========================================

void loop() {

    if (!modemReady) {
        delay(1000);
        return;
    }

    // Kontrollera efter nya SMS med jämna intervall
    if (millis() - lastSmsCheck >= SMS_CHECK_INTERVAL) {

        lastSmsCheck = millis();

        processIncomingSms();
    }

    delay(10);
}
