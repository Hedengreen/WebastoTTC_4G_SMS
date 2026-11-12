
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

// Ändra till ditt eget nummer.
// Internationellt format, exempelvis +46701234567.
#define AUTHORIZED_PHONE "+46706325326"

// Kontrollintervall för SMS
#define SMS_CHECK_INTERVAL 3000
