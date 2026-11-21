
#include "wbus.h"

// =====================================================
// W-BUS CONFIGURATION
// =====================================================

#define WBUS_BAUDRATE 2400

// Typical W-Bus addresses
#define WBUS_TX_HEADER 0xF4
#define WBUS_RX_HEADER 0x4F

// =====================================================
// INTERNAL STATE
// =====================================================

static HardwareSerial* wbusSerial = nullptr;

static uint8_t rxBuffer[260];
static uint16_t rxIndex = 0;

static bool packetReceived = false;
static String lastPacket = "";

static uint8_t expectedLength = 0;

// =====================================================
// INITIALIZATION
// =====================================================

void wbusInit(HardwareSerial& serial, int rxPin, int txPin)
{
    wbusSerial = &serial;

    wbusSerial->begin(
        WBUS_BAUDRATE,
        SERIAL_8E1,
        rxPin,
        txPin
    );

    rxIndex = 0;
    packetReceived = false;
    lastPacket = "";

    Serial.println("W-Bus UART initialized.");
    Serial.println("Baudrate: 2400");
    Serial.println("Format: 8E1");
    Serial.println("TX disabled - receive/parser only.");
}

// =====================================================
// PACKET CHECKSUM
// =====================================================

static bool validateChecksum()
{
    if (rxIndex < 4)
        return false;

    uint8_t checksum = 0;

    for (uint16_t i = 0; i < rxIndex - 1; i++)
    {
        checksum ^= rxBuffer[i];
    }

    return checksum == rxBuffer[rxIndex - 1];
}

// =====================================================
// PACKET PROCESSING
// =====================================================

void wbusProcess()
{
    if (wbusSerial == nullptr)
        return;

    while (wbusSerial->available())
    {
        uint8_t incoming = wbusSerial->read();

        // Look for a recognized header
        if (rxIndex == 0)
        {
            if (incoming == WBUS_RX_HEADER ||
                incoming == WBUS_TX_HEADER)
            {
                rxBuffer[rxIndex++] = incoming;
            }

            continue;
        }

        // Prevent buffer overflow
        if (rxIndex >= sizeof(rxBuffer))
        {
            rxIndex = 0;
            expectedLength = 0;
            continue;
        }

        rxBuffer[rxIndex++] = incoming;

        // Second byte is packet length
        if (rxIndex == 2)
        {
            expectedLength = incoming;

            if (expectedLength < 2 ||
                expectedLength > 255)
            {
                rxIndex = 0;
                expectedLength = 0;
                continue;
            }
        }

        // Total packet length = header + length byte + length field
        if (rxIndex >= 2 &&
            expectedLength > 0 &&
            rxIndex == expectedLength + 2)
        {
            if (validateChecksum())
            {
                packetReceived = true;

                lastPacket = "";

                for (uint16_t i = 0; i < rxIndex; i++)
                {
                    char buffer[5];

                    sprintf(buffer, "%02X", rxBuffer[i]);

                    if (i > 0)
                        lastPacket += " ";

                    lastPacket += buffer;
                }

                Serial.print("W-Bus RX: ");
                Serial.println(lastPacket);
            }
            else
            {
                Serial.println("W-Bus: Invalid checksum.");
            }

            rxIndex = 0;
            expectedLength = 0;
        }
    }
}

// =====================================================
// STATUS
// =====================================================

bool wbusHasReceivedPacket()
{
    return packetReceived;
}

String wbusGetLastPacket()
{
    return lastPacket;
}

String wbusGetStatus()
{
    if (packetReceived)
        return "Valid W-Bus packet received: " + lastPacket;

    return "No valid W-Bus packet received.";
}
