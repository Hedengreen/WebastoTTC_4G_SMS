
#pragma once

#include <Arduino.h>

// Initialize W-Bus UART
void wbusInit(HardwareSerial& serial, int rxPin, int txPin);

// Process incoming W-Bus data
void wbusProcess();

// Check whether a valid W-Bus packet has been received
bool wbusHasReceivedPacket();

// Get last received packet
String wbusGetLastPacket();

// Get diagnostic information
String wbusGetStatus();
