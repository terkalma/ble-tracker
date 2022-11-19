#pragma once
#include <Arduino.h>

// Function Code
constexpr byte FN_RANGING_INITIATION = 0x20;
constexpr byte FN_POLL = 0x61;
constexpr byte FN_RESPONSE = 0x50;
constexpr byte FN_FINAL_RESPONSE = 0x69;

constexpr byte SHORT_SRC_AND_DEST = 0x88;
constexpr byte SHORT_SRC_LONG_DEST = 0x8C;
constexpr byte DATA = 0x41;

/* Application ID */
constexpr byte RTLS_APP_ID_LOW = 0x9A;
constexpr byte RTLS_APP_ID_HIGH = 0x60;
constexpr uint16_t RTLS_APP_ID = RTLS_APP_ID_LOW | ((uint16_t) (RTLS_APP_ID_HIGH << 8));
constexpr byte tag_address_start[8] = {0x00, 0x01, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x70};

namespace STwr {
    void printMessage(uint16_t length, uint8_t *msg_bytes);
    byte increaseSequenceNumber();
    void waitForTransmission();
    boolean receiveFrame();

    // tag functions
    void transmitPoll(byte anchor_id);
    void transmitFinal(byte anchor_id);

    // anchor functions
    void transmitRangeInit(byte destination);
    void transmitResponseToPoll(byte destination);
}