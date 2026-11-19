
#pragma once

#include <Arduino.h>

// Information från GPS-mottagaren.
struct GpsData {
    double latitude = 0.0;
    double longitude = 0.0;

    double altitude = 0.0;
    double speedKmph = 0.0;

    uint32_t satellites = 0;

    bool valid = false;
};

// Startar GPS med angiven UART och pinnar.
void gpsBegin(
    HardwareSerial& serial,
    int rxPin,
    int txPin
);

// Läser inkommande GPS-data.
// Ska anropas regelbundet från loop().
void gpsUpdate();

// Hämtar senast tillgängliga GPS-information.
GpsData gpsGetData();

// Skapar ett SMS med GPS-positionen.
String gpsGetSmsResponse();
