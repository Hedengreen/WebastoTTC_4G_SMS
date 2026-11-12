
#include <Arduino.h>

#include "sms_handler.h"
#include "secrets.h"

// --------------------------------------------------
// Hjälpfunktioner
// --------------------------------------------------

// Normaliserar telefonnummer så att olika skrivsätt
// kan jämföras med varandra.
static String normalizeNumber(String number) {
    number.trim();

    number.replace(" ", "");
    number.replace("-", "");
    number.replace("(", "");
    number.replace(")", "");

    // 0046... blir +46...
    if (number.startsWith("00")) {
        number = "+" + number.substring(2);
    }

    return number;
}

// Gör om SMS-texten till versaler och tar bort
// onödiga mellanslag i början och slutet.
static String normalizeMessage(String message) {
    message.trim();
    message.toUpperCase();

    return message;
}

// Kontrollerar att en text endast innehåller siffror.
static bool containsOnlyDigits(const String& value) {
    if (value.length() == 0) {
        return false;
    }

    for (unsigned int i = 0; i < value.length(); i++) {
        if (!isDigit(value[i])) {
            return false;
        }
    }

    return true;
}

// --------------------------------------------------
// Kontroll av behörigt telefonnummer
// --------------------------------------------------

bool isAuthorizedNumber(const String& phoneNumber) {
    String receivedNumber = normalizeNumber(phoneNumber);
    String authorizedNumber = normalizeNumber(
        String(AUTHORIZED_PHONE)
    );

    return receivedNumber == authorizedNumber;
}

// --------------------------------------------------
// Tolkning av SMS-kommandon
// --------------------------------------------------

SmsCommand parseSmsCommand(const String& message) {
    SmsCommand result;

    String command = normalizeMessage(message);

    // PING
    if (command == "PING") {
        result.type = SmsCommandType::Ping;
        result.valid = true;
        return result;
    }

    // HELP
    if (command == "HELP") {
        result.type = SmsCommandType::Help;
        result.valid = true;
        return result;
    }

    // STATUS
    if (command == "STATUS") {
        result.type = SmsCommandType::Status;
        result.valid = true;
        return result;
    }

    // STOP
    if (command == "STOP") {
        result.type = SmsCommandType::Stop;
        result.valid = true;
        return result;
    }

    // GPS
    if (command == "GPS") {
        result.type = SmsCommandType::Gps;
        result.valid = true;
        return result;
    }

    // START utan angiven tid
    if (command == "START") {
        result.type = SmsCommandType::Start;
        result.durationMinutes = 30;
        result.valid = true;
        return result;
    }

    // START med angiven tid, exempelvis START 45
    if (command.startsWith("START ")) {
        String duration = command.substring(6);
        duration.trim();

        if (!containsOnlyDigits(duration)) {
            return result;
        }

        // Begränsa längden innan vi konverterar till heltal.
        if (duration.length() > 3) {
            return result;
        }

        int minutes = duration.toInt();

        if (minutes < 1 || minutes > 120) {
            return result;
        }

        result.type = SmsCommandType::Start;
        result.durationMinutes = minutes;
        result.valid = true;

        return result;
    }

    // Okänt eller felaktigt kommando
    return result;
}

// --------------------------------------------------
// Skapa SMS-svar
// --------------------------------------------------

String getCommandResponse(const SmsCommand& command) {

    if (!command.valid) {
        return "Okant kommando. Skicka HELP for hjalp.";
    }

    switch (command.type) {

        case SmsCommandType::Ping:
            return "Webasto-enheten svarar. SMS fungerar.";

        case SmsCommandType::Help:
            return
                "Webasto SMS-kommandon:\n"
                "PING - Kontrollera anslutning\n"
                "STATUS - Hamta status\n"
                "START - Starta 30 min\n"
                "START 45 - Starta 45 min\n"
                "STOP - Stoppa varmare\n"
                "GPS - Hamta position";

        case SmsCommandType::Status:
            return "Statusfunktion kommer snart.";

        case SmsCommandType::Start:
            return
                "START " +
                String(command.durationMinutes) +
                " min mottaget. "
                "Varmerstyrning ar annu inte aktiverad.";

        case SmsCommandType::Stop:
            return
                "STOP mottaget. "
                "Varmerstyrning ar annu inte aktiverad.";

        case SmsCommandType::Gps:
            return "GPS-funktionen är ännu inte aktiverad.";

        default:
            return "Okant kommando. Skicka HELP för hjälp.";
    }
}
