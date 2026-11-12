
#pragma once

// ========================================
// MODEM - LILYGO T-A7670E R2
// ========================================

#define MODEM_BAUDRATE 115200

#define MODEM_RX_PIN 27
#define MODEM_TX_PIN 26

#define MODEM_POWERON_PIN 12
#define MODEM_PWRKEY_PIN 4
#define MODEM_DTR_PIN 25

// ========================================
// SMS SETTINGS
// ========================================

// Kontrollintervall för SMS
#define SMS_CHECK_INTERVAL 3000


#define WBUS_ADDR_CONTROLLER 0xF
#define WBUS_ADDR_HEATER     0x4

// UART pins - preliminary
#define WBUS_TX_PIN 22
#define WBUS_RX_PIN 23

// No external enable pin for now
#define WBUS_EN_PIN -1

// Disable automatic break until hardware is verified
#define WBUS_SEND_BREAK 0