
#pragma once

#include <Arduino.h>

// Alla kommandon som SMS-hanteraren kan känna igen.
enum class SmsCommandType {
    Unknown,
    Ping,
    Help,
    Status,
    Start,
    Stop,
    Gps
};

// Informationen som skickas vidare efter att ett SMS har tolkats.
struct SmsCommand {
    SmsCommandType type = SmsCommandType::Unknown;
    uint16_t durationMinutes = 30;
    bool valid = false;
};

// Kontrollerar om telefonnumret får styra Webasto.
bool isAuthorizedNumber(const String& phoneNumber);

// Tolkar innehållet i ett SMS och returnerar ett kommando.
SmsCommand parseSmsCommand(const String& message);

// Skapar ett svar baserat på det tolkade kommandot.
String getCommandResponse(const SmsCommand& command);
