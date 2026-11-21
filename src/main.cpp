
#define TINY_GSM_MODEM_SIM7600

#include <Arduino.h>
#include <TinyGsmClient.h>

#include "heater.h"
#include "sms.h"
#include "secrets.h"

// =====================================================
// HARDWARE - LilyGO A7670G
// =====================================================

#define MODEM_RX_PIN 27
#define MODEM_TX_PIN 26

#define MODEM_DTR_PIN 25
#define MODEM_PWRKEY_PIN 4
#define MODEM_POWERON_PIN 12
#define MODEM_RESET_PIN 5

#define MODEM_RESET_LEVEL HIGH

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);

// =====================================================
// MODEM POWER SEQUENCE
// =====================================================

void modemPowerOn()
{
    pinMode(MODEM_POWERON_PIN, OUTPUT);
    digitalWrite(MODEM_POWERON_PIN, HIGH);

    pinMode(MODEM_RESET_PIN, OUTPUT);
    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);
    delay(100);

    digitalWrite(MODEM_RESET_PIN, MODEM_RESET_LEVEL);
    delay(2600);

    digitalWrite(MODEM_RESET_PIN, !MODEM_RESET_LEVEL);

    pinMode(MODEM_DTR_PIN, OUTPUT);
    digitalWrite(MODEM_DTR_PIN, LOW);

    pinMode(MODEM_PWRKEY_PIN, OUTPUT);
    digitalWrite(MODEM_PWRKEY_PIN, LOW);
    delay(100);

    digitalWrite(MODEM_PWRKEY_PIN, HIGH);
    delay(100);

    digitalWrite(MODEM_PWRKEY_PIN, LOW);
}

// =====================================================
// AT COMMAND HELPERS
// =====================================================

String readModemResponse(unsigned long timeout = 3000)
{
    String response;
    unsigned long start = millis();

    while (millis() - start < timeout)
    {
        while (SerialAT.available())
        {
            response += (char)SerialAT.read();
        }

        delay(10);
    }

    return response;
}

String sendATCommand(const String& command, unsigned long timeout = 3000)
{
    while (SerialAT.available())
    {
        SerialAT.read();
    }

    SerialAT.println(command);

    return readModemResponse(timeout);
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("==============================");
    Serial.println("Webasto SMS Controller");
    Serial.println("==============================");

    // -------------------------------------------------
    // UART
    // -------------------------------------------------

    SerialAT.begin(
        115200,
        SERIAL_8N1,
        MODEM_RX_PIN,
        MODEM_TX_PIN
    );

    // -------------------------------------------------
    // POWER UP MODEM
    // -------------------------------------------------

    modemPowerOn();
    delay(3000);

    // -------------------------------------------------
    // MODEM INITIALIZATION
    // -------------------------------------------------

    Serial.println("Initializing modem...");

    if (!modem.init())
    {
        Serial.println("Modem initialization failed!");
        return;
    }

    Serial.println("Modem initialized.");
    Serial.println(modem.getModemInfo());

    // -------------------------------------------------
    // NETWORK
    // -------------------------------------------------

    Serial.println("Waiting for network...");

    if (modem.waitForNetwork(60000L))
    {
        Serial.println("Network registered.");
    }
    else
    {
        Serial.println("Network registration failed.");
    }

    // -------------------------------------------------
    // SMS INITIALIZATION
    // -------------------------------------------------

    smsInit();

    // -------------------------------------------------
    // READY
    // -------------------------------------------------

    Serial.println();
    Serial.println("SMS controller ready.");
    Serial.println("Heater control disabled.");
    Serial.println("W-Bus inactive.");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    smsCheck();

    delay(5000);
}
