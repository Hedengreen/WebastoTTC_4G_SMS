
#define TINY_GSM_MODEM_SIM7600
#include "heater.h"

#include <Arduino.h>
#include <TinyGsmClient.h>
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
// STATE
// =====================================================



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
// SMS SENDING
// =====================================================

bool sendSMS(String number, String message)
{
    Serial.println("Sending SMS to: " + number);

    while (SerialAT.available())
    {
        SerialAT.read();
    }

    SerialAT.print("AT+CMGS=\"");
    SerialAT.print(number);
    SerialAT.println("\"");

    String response = "";

    unsigned long start = millis();

    // Wait for SMS prompt
    while (millis() - start < 5000)
    {
        while (SerialAT.available())
        {
            char c = SerialAT.read();
            response += c;

            if (c == '>')
            {
                SerialAT.print(message);
                SerialAT.write(26); // Ctrl+Z

                String result = readModemResponse(15000);

                Serial.println(result);

                return result.indexOf("+CMGS:") >= 0;
            }
        }

        delay(10);
    }

    Serial.println("SMS prompt timeout.");
    Serial.println(response);

    return false;
}

// =====================================================
// AUTHORIZATION
// =====================================================

bool isAuthorizedNumber(String number)
{
    number.trim();

    for (int i = 0; i < AUTHORIZED_NUMBER_COUNT; i++)
    {
        if (number == AUTHORIZED_NUMBERS[i])
        {
            return true;
        }
    }

    return false;
}



// =====================================================
// SMS COMMAND PROCESSING
// =====================================================

void processSMS(String sender, String message)
{
    sender.trim();
    message.trim();
    message.toUpperCase();

    Serial.println();
    Serial.println("SMS received");
    Serial.println("Sender: " + sender);

    // -------------------------------------------------
    // AUTHORIZATION FIRST
    // -------------------------------------------------

    if (!isAuthorizedNumber(sender))
    {
        Serial.println("Unauthorized sender.");
        return;
    }

    Serial.println("Authorized sender.");

    // -------------------------------------------------
    // STRICT COMMAND MATCHING
    // -------------------------------------------------

    if (message == "START 30")
    {
        startHeater(30);

        sendSMS(
            sender,
            "START 30 received. Heater control is disabled."
        );

        return;
    }

    if (message == "START 60")
    {
        startHeater(60);

        sendSMS(
            sender,
            "START 60 received. Heater control is disabled."
        );

        return;
    }

    if (message == "STOP")
    {
        stopHeater();

        sendSMS(
            sender,
            "STOP received. Heater control is disabled."
        );

        return;
    }

    if (message == "STATUS")
    {
        sendSMS(sender, getStatus());
        return;
    }

    if (message == "HELP")
    {
        sendSMS(
            sender,
            "Commands:\n"
            "START 30\n"
            "START 60\n"
            "STOP\n"
            "STATUS\n"
            "Heater control is currently disabled."
        );

        return;
    }

    // -------------------------------------------------
    // INVALID COMMAND
    // -------------------------------------------------

    sendSMS(
        sender,
        "Invalid command.\nSend HELP."
    );

    Serial.println("Invalid command.");
}

// =====================================================
// SMS READING
// =====================================================

void checkSMS()
{
    String response = sendATCommand("AT+CMGL=\"REC UNREAD\"", 5000);

    Serial.println("SMS list:");
    Serial.println(response);

    int position = 0;

    while (true)
    {
        int headerStart = response.indexOf("+CMGL:", position);

        if (headerStart < 0)
        {
            break;
        }

        int headerEnd = response.indexOf('\n', headerStart);

        if (headerEnd < 0)
        {
            break;
        }

        String header = response.substring(headerStart, headerEnd);
        header.trim();

        // Extract SMS index
        int colon = header.indexOf(':');
        int comma = header.indexOf(',', colon);

        if (colon < 0 || comma < 0)
        {
            position = headerEnd + 1;
            continue;
        }

        int smsIndex = header.substring(colon + 1, comma).toInt();

        // Extract sender number from quoted fields
        int firstQuote = header.indexOf('"');
        int secondQuote = header.indexOf('"', firstQuote + 1);
        int thirdQuote = header.indexOf('"', secondQuote + 1);
        int fourthQuote = header.indexOf('"', thirdQuote + 1);

        if (thirdQuote < 0 || fourthQuote < 0)
        {
            position = headerEnd + 1;
            continue;
        }

        String sender = header.substring(thirdQuote + 1, fourthQuote);

        // SMS body is the next line
        int bodyStart = headerEnd + 1;

        while (bodyStart < response.length() &&
               (response.charAt(bodyStart) == '\r' ||
                response.charAt(bodyStart) == '\n'))
        {
            bodyStart++;
        }

        int bodyEnd = response.indexOf('\n', bodyStart);

        if (bodyEnd < 0)
        {
            bodyEnd = response.length();
        }

        String message = response.substring(bodyStart, bodyEnd);
        message.trim();

        Serial.println("SMS index: " + String(smsIndex));
        Serial.println("Sender: " + sender);
        Serial.println("Body: " + message);

        processSMS(sender, message);

        // Delete processed SMS
        sendATCommand("AT+CMGD=" + String(smsIndex));

        position = bodyEnd + 1;
    }
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

    SerialAT.begin(
        115200,
        SERIAL_8N1,
        MODEM_RX_PIN,
        MODEM_TX_PIN
    );

    modemPowerOn();
    delay(3000);

    Serial.println("Initializing modem...");

    if (!modem.init())
    {
        Serial.println("Modem initialization failed!");
        return;
    }

    Serial.println("Modem initialized.");

    Serial.println(modem.getModemInfo());

    Serial.println("Waiting for network...");

    if (modem.waitForNetwork(60000L))
    {
        Serial.println("Network registered.");
    }
    else
    {
        Serial.println("Network registration failed.");
    }

    // SMS text mode
    Serial.println(sendATCommand("AT+CMGF=1"));

    // Notify on new SMS
    Serial.println(sendATCommand("AT+CNMI=2,1,0,0,0"));

    Serial.println();
    Serial.println("SMS controller ready.");
    Serial.println("W-Bus inactive.");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    checkSMS();

    delay(5000);
}
