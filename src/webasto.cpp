
#include <Arduino.h>
#include "wbus_simple.h"
#include "project_config.h"

HardwareSerial WBusSerial(2);

WBusSimple webasto(WBusSerial);

void setupWebasto() {
    WBusSerial.begin(
        2400,
        SERIAL_8E1,
        WBUS_RX_PIN,
        WBUS_TX_PIN
    );

    if (webasto.begin()) {
        Serial.println("W-Bus initialized");
    } else {
        Serial.println("W-Bus initialization failed");
    }
}

void testWebastoStart() {
    Serial.println("Testing Webasto START");

    bool success = webasto.startParkingHeater(30);

    Serial.println(success
        ? "START command acknowledged"
        : "START command failed");
}

void testWebastoStop() {
    Serial.println("Testing Webasto STOP");

    bool success = webasto.stop();

    Serial.println(success
        ? "STOP command acknowledged"
        : "STOP command failed");
}
