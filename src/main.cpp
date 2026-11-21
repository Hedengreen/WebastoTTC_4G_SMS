
#define TINY_GSM_MODEM_SIM7600

#include <Arduino.h>
#include <TinyGsmClient.h>

// LilyGO T-A7670G / ESP32
#define MODEM_RX_PIN 27
#define MODEM_TX_PIN 26

#define MODEM_DTR_PIN 25
#define MODEM_PWRKEY_PIN 4
#define MODEM_POWERON_PIN 12
#define MODEM_RESET_PIN 5

#define MODEM_RESET_LEVEL HIGH

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);

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

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Starting Webasto SMS controller");

    SerialAT.begin(115200, SERIAL_8N1, MODEM_RX_PIN, MODEM_TX_PIN);

    modemPowerOn();
    delay(3000);

    Serial.println("Initializing TinyGSM...");

    if (!modem.init())
    {
        Serial.println("TinyGSM initialization failed!");
        return;
    }

    Serial.println("TinyGSM initialized!");

    Serial.print("Modem: ");
    Serial.println(modem.getModemName());

    Serial.println();
    Serial.println("Checking SIM...");

    if (modem.getSimStatus() == SIM_READY)
    {
        Serial.println("SIM card is ready.");
    }
    else
    {
        Serial.println("SIM card is not ready.");
    }

    Serial.println();
    Serial.println("Waiting for network...");

    if (modem.waitForNetwork(60000L))
    {
        Serial.println("Network registered!");

        Serial.print("Signal quality: ");
        Serial.println(modem.getSignalQuality());

        Serial.print("Network connected: ");
        Serial.println(modem.isNetworkConnected());
    }
    else
    {
        Serial.println("Network registration failed.");

        Serial.print("Signal quality: ");
        Serial.println(modem.getSignalQuality());
    }

    Serial.println();
    Serial.println("Diagnostic complete.");
}

void loop()
{
}
