
#pragma once

#include <Arduino.h>

// Information om ett mottaget SMS.
struct SmsMessage {
    int index = -1;
    String sender;
    String text;
};

// Startar modemet och förbereder SMS-funktionen.
bool modemBegin();

// Skickar ett SMS till angivet telefonnummer.
bool modemSendSms(
    const String& phoneNumber,
    const String& message
);

// Läser nästa olästa SMS.
// Returnerar true om ett SMS hittades.
bool modemReadNextSms(SmsMessage& sms);

// Raderar ett SMS från modemets minne.
bool modemDeleteSms(int index);
