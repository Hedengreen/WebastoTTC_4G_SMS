
#include <Arduino.h>
#include <TinyGPSPlus.h>

#include "gps.h"

namespace {

TinyGPSPlus gps;
HardwareSerial* gpsSerial = nullptr;

}

// ========================================
// STARTA GPS
// ========================================

void gpsBegin(
    HardwareSerial& serial,
    int rxPin,
    int txPin
) {
    gpsSerial = &serial;

    gpsSerial->begin(
        9600,
        SERIAL_8N1,
        rxPin,
        txPin
    );

    Serial.println("GPS initialized.");
}


// ========================================
// LÄS GPS-DATA
// ========================================

void gpsUpdate() {

    if (gpsSerial == nullptr) {
        return;
    }

    while (gpsSerial->available()) {
        gps.encode(gpsSerial->read());
    }
}


// ========================================
// HÄMTA GPS-INFORMATION
// ========================================

GpsData gpsGetData() {

    GpsData data;

    data.valid = gps.location.isValid() &&
                 gps.location.age() < 10000;

    if (!data.valid) {
        return data;
    }

    data.latitude = gps.location.lat();
    data.longitude = gps.location.lng();

    if (gps.altitude.isValid()) {
        data.altitude = gps.altitude.meters();
    }

    if (gps.speed.isValid()) {
        data.speedKmph = gps.speed.kmph();
    }

    if (gps.satellites.isValid()) {
        data.satellites = gps.satellites.value();
    }

    return data;
}


// ========================================
// SKAPA SMS-SVAR
// ========================================

String gpsGetSmsResponse() {

    GpsData data = gpsGetData();

    if (!data.valid) {
        return "GPS-position saknas. "
               "Kontrollera mottagning och satellitsignal.";
    }

    String response;

    response += "Webasto GPS-position:\n";

    response += "Lat: ";
    response += String(data.latitude, 6);

    response += "\nLon: ";
    response += String(data.longitude, 6);

    response += "\nSatelliter: ";
    response += String(data.satellites);

    response += "\nhttps://maps.google.com/?q=";
    response += String(data.latitude, 6);
    response += ",";
    response += String(data.longitude, 6);

    return response;
}
