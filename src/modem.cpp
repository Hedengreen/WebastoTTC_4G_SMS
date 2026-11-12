
#include <Arduino.h>

#include "modem.h"

// SerialAT skapas i main.cpp.
// Vi använder samma UART-instans här.
extern HardwareSerial SerialAT;

namespace {

// Skickar ett AT-kommando och samlar in svaret.
String sendATCommand(
    const String& command,
    uint32_t timeoutMs = 5000
) {
    // Rensa gammal data.
    while (SerialAT.available()) {
        SerialAT.read();
    }

    SerialAT.print(command);
    SerialAT.print("\r\n");

    String response;
    uint32_t started = millis();

    while (millis() - started < timeoutMs) {
        while (SerialAT.available()) {
            response += (char)SerialAT.read();
        }

        if (response.indexOf("\r\nOK\r\n") >= 0 ||
            response.endsWith("\r\nOK") ||
            response.indexOf("\r\nERROR\r\n") >= 0 ||
            response.indexOf("+CME ERROR:") >= 0) {
            break;
        }

        delay(1);
    }

    Serial.println("AT: " + command);
    Serial.println(response);

    return response;
}

// Kontrollerar om ett AT-svar innehåller OK.
bool responseOK(const String& response) {
    return response.indexOf("\r\nOK") >= 0 ||
           response.endsWith("OK");
}

// Hämtar telefonnumret från en +CMGL-header.
String extractPhoneNumber(const String& header) {
    int firstQuote = header.indexOf('"');

    if (firstQuote < 0) {
        return "";
    }

    int secondQuote = header.indexOf('"', firstQuote + 1);

    if (secondQuote < 0) {
        return "";
    }

    int thirdQuote = header.indexOf('"', secondQuote + 1);

    if (thirdQuote < 0) {
        return "";
    }

    int fourthQuote = header.indexOf('"', thirdQuote + 1);

    if (fourthQuote < 0) {
        return "";
    }

    return header.substring(thirdQuote + 1, fourthQuote);
}

} // namespace


// --------------------------------------------------
// Initiering av SMS-funktionen
// --------------------------------------------------

bool modemBegin() {
    Serial.println("Configuring SMS modem...");

    bool success = true;

    success &= responseOK(sendATCommand("AT+CMGF=1"));
    success &= responseOK(sendATCommand("AT+CSCS=\"GSM\""));
    success &= responseOK(sendATCommand("AT+CNMI=2,1,0,0,0"));

    if (success) {
        Serial.println("SMS modem initialized.");
    } else {
        Serial.println("SMS modem initialization failed.");
    }

    return success;
}


// --------------------------------------------------
// Skicka SMS
// --------------------------------------------------

bool modemSendSms(
    const String& phoneNumber,
    const String& message
) {
    Serial.println("Sending SMS to: " + phoneNumber);

    // Rensa gammal data.
    while (SerialAT.available()) {
        SerialAT.read();
    }

    SerialAT.print("AT+CMGS=\"");
    SerialAT.print(phoneNumber);
    SerialAT.print("\"\r\n");

    // Vänta på modemets SMS-prompt.
    String response;
    uint32_t started = millis();

    while (millis() - started < 5000) {
        while (SerialAT.available()) {
            char c = SerialAT.read();
            response += c;

            if (c == '>') {
                goto promptReceived;
            }
        }

        delay(1);
    }

    Serial.println("SMS prompt timeout: " + response);
    return false;

promptReceived:

    // Skicka meddelandet.
    SerialAT.print(message);

    // Ctrl+Z avslutar SMS-meddelandet.
    SerialAT.write(26);

    response = "";
    started = millis();

    while (millis() - started < 30000) {
        while (SerialAT.available()) {
            response += (char)SerialAT.read();
        }

        if (response.indexOf("+CMGS:") >= 0 &&
            response.indexOf("OK") >= 0) {
            Serial.println("SMS sent successfully.");
            return true;
        }

        if (response.indexOf("ERROR") >= 0) {
            Serial.println("SMS sending failed: " + response);
            return false;
        }

        delay(1);
    }

    Serial.println("SMS sending timeout: " + response);
    return false;
}


// --------------------------------------------------
// Läs nästa olästa SMS
// --------------------------------------------------

bool modemReadNextSms(SmsMessage& sms) {
    String response = sendATCommand(
        "AT+CMGL=\"REC UNREAD\"",
        10000
    );

    int headerStart = response.indexOf("+CMGL:");

    if (headerStart < 0) {
        return false;
    }

    int headerEnd = response.indexOf('\n', headerStart);

    if (headerEnd < 0) {
        return false;
    }

    String header = response.substring(headerStart, headerEnd);
    header.trim();

    // Hämta SMS-index.
    int colon = header.indexOf(':');

    if (colon < 0) {
        return false;
    }

    String indexString = header.substring(colon + 1);
    indexString.trim();

    int comma = indexString.indexOf(',');

    if (comma < 0) {
        return false;
    }

    sms.index = indexString.substring(0, comma).toInt();

    // Hämta avsändarens telefonnummer.
    sms.sender = extractPhoneNumber(header);

    // Nästa rad innehåller SMS-texten.
    int bodyStart = headerEnd + 1;

    while (bodyStart < response.length() &&
           (response[bodyStart] == '\r' ||
            response[bodyStart] == '\n')) {
        bodyStart++;
    }

    int bodyEnd = response.indexOf('\n', bodyStart);

    if (bodyEnd < 0) {
        bodyEnd = response.length();
    }

    sms.text = response.substring(bodyStart, bodyEnd);
    sms.text.trim();

    if (sms.index < 0 || sms.sender.length() == 0) {
        return false;
    }

    Serial.println("SMS received!");
    Serial.println("From: " + sms.sender);
    Serial.println("Message: " + sms.text);

    return true;
}


// --------------------------------------------------
// Radera SMS från modemets minne
// --------------------------------------------------

bool modemDeleteSms(int index) {
    if (index < 0) {
        return false;
    }

    String command = "AT+CMGD=" + String(index);

    String response = sendATCommand(command);

    return responseOK(response);
}
