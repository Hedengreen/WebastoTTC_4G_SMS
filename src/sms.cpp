#include "sms.h"
#include "secrets.h"
#include "heater.h"

#include <Arduino.h>

// =====================================================
// SHARED MODEM INTERFACE
// =====================================================

// SerialAT is defined in main.cpp
extern HardwareSerial SerialAT;

// sendATCommand is defined in main.cpp
String sendATCommand(const String& command, unsigned long timeout);

// =====================================================
// SMS SENDING
// =====================================================

static bool sendSMS(String number, String message)
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

                String result = sendATCommand("", 15000);

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

static bool isAuthorizedNumber(String number)
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

static void processSMS(String sender, String message)
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
// SMS INITIALIZATION
// =====================================================

void smsInit()
{
    Serial.println("Initializing SMS...");

    // SMS text mode
    Serial.println(sendATCommand("AT+CMGF=1", 3000));

    // Notify on new SMS
    Serial.println(sendATCommand("AT+CNMI=2,1,0,0,0", 3000));

    Serial.println("SMS module initialized.");
}

// =====================================================
// SMS READING
// =====================================================

void smsCheck()
{
    String response = sendATCommand(
        "AT+CMGL=\"REC UNREAD\"",
        5000
    );

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

        if (firstQuote < 0 ||
            secondQuote < 0 ||
            thirdQuote < 0 ||
            fourthQuote < 0)
        {
            position = headerEnd + 1;
            continue;
        }

        String sender = header.substring(
            thirdQuote + 1,
            fourthQuote
        );

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
        sendATCommand(
            "AT+CMGD=" + String(smsIndex),
            3000
        );

        position = bodyEnd + 1;
    }
}