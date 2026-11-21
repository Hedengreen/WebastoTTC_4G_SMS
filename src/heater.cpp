
#include "heater.h"

// =====================================================
// HEATER STATE
// =====================================================

// Heater control is currently disabled.
// These variables are internal to this module.

static bool heaterRunning = false;
static bool startRequested = false;
static int requestedRunTime = 0;
static bool wbusEnabled = false;

// =====================================================
// START
// =====================================================

void startHeater(int minutes)
{
    // SAFETY:
    // Heater activation is disabled.
    // No W-Bus telegrams are sent.

    startRequested = false;
    heaterRunning = false;
    requestedRunTime = 0;

    Serial.println("START command accepted for testing.");
    Serial.println("Heater activation is disabled.");
}

// =====================================================
// STOP
// =====================================================

void stopHeater()
{
    // Logical stop only.
    // W-Bus remains inactive.

    startRequested = false;
    heaterRunning = false;
    requestedRunTime = 0;

    Serial.println("STOP command received.");
    Serial.println("W-Bus is inactive.");
}

// =====================================================
// STATUS
// =====================================================

String getStatus()
{
    String status = "Webasto SMS Controller\n";

    status += "Modem: Initialized\n";
    status += "Heater feedback: Unavailable\n";
    status += "Heater control: Disabled\n";
    status += "W-Bus: Inactive\n";

    return status;
}
