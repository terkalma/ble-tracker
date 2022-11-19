#include "STwr.h"
#include "DW1000Ng.hpp"
#include "DW1000NgTime.hpp"
#include "DW1000NgUtils.hpp"

static byte SEQ_NUMBER = 0;

namespace STwr {

    void printMessage(uint16_t length, uint8_t *msg_bytes) {
        for (int i=0; i<length; i++) {
            printf("%02X:", msg_bytes[i]);
        }
        printf("\n");
    }

    byte increaseSequenceNumber()
    {
        return ++SEQ_NUMBER;
    }

    void waitForTransmission()
    {
        while(!DW1000Ng::isTransmitDone()) {
            #if defined(ESP8266)
            yield();
            #endif
        }
        DW1000Ng::clearTransmitStatus();
    }

    boolean receiveFrame()
    {
        DW1000Ng::startReceive();
        while(!DW1000Ng::isReceiveDone()) {
            if(DW1000Ng::isReceiveTimeout() ) {
                DW1000Ng::clearReceiveTimeoutStatus();
                return false;
            }
        }
        DW1000Ng::clearReceiveStatus();
        return true;
    }

    /**
     *
     * Tag Functions
     *
     *
    */
    void transmitPoll(byte anchor_id)
    {
        byte poll_data[] = {
            DATA,
            SHORT_SRC_AND_DEST,
            STwr::increaseSequenceNumber(),
            0,0, // PAN ID
            0,0, // Target Address
            0,0, // Source Address
            FN_POLL // POLL msg
        };
        DW1000Ng::getNetworkId(&poll_data[3]);
        poll_data[5] = anchor_id;
        DW1000Ng::getDeviceAddress(&poll_data[7]);
        DW1000Ng::setTransmitData(poll_data, sizeof(poll_data));
        DW1000Ng::startTransmit();
        STwr::waitForTransmission();
    }

    void transmitFinal(byte anchor_id)
    {
        /* Calculation of future time */
        byte future_time_bytes[LENGTH_TIMESTAMP];
        uint64_t reply_delay_micros = 1500;

        uint64_t time_final_message_sent = DW1000Ng::getSystemTimestamp();
        time_final_message_sent += DW1000NgTime::microsecondsToUWBTime(reply_delay_micros);
        DW1000NgUtils::writeValueToBytes(future_time_bytes, time_final_message_sent, LENGTH_TIMESTAMP);
        DW1000Ng::setDelayedTRX(future_time_bytes);
        time_final_message_sent += DW1000Ng::getTxAntennaDelay();

        byte final_message[] = {
            DATA,
            SHORT_SRC_AND_DEST,
            STwr::increaseSequenceNumber(),
            0,0, // PAN
            0,0, // Target
            0,0, // Source
            FN_FINAL_RESPONSE, // Final Response Function Code
            0,0,0,0, // Poll Sent TS
            0,0,0,0, // Response Received TS
            0,0,0,0  // Final Message Sent TS
        };

        DW1000Ng::getNetworkId(&final_message[3]);
        final_message[5] = anchor_id;
        DW1000Ng::getDeviceAddress(&final_message[7]);

        DW1000NgUtils::writeValueToBytes(final_message + 10, (uint32_t) DW1000Ng::getTransmitTimestamp(), 4);
        DW1000NgUtils::writeValueToBytes(final_message + 14, (uint32_t) DW1000Ng::getReceiveTimestamp(), 4);
        DW1000NgUtils::writeValueToBytes(final_message + 18, (uint32_t) time_final_message_sent, 4);
        DW1000Ng::setTransmitData(final_message, sizeof(final_message));
        DW1000Ng::startTransmit(TransmitMode::DELAYED);
        // printMessage(18, final_message);
        STwr::waitForTransmission();
    }

    /**
     *
     * Anchor functions
     *
     *
    */
    void transmitRangeInit(byte destination)
    {
        byte RangingInitiation[] = {
            DATA,
            SHORT_SRC_LONG_DEST, // Range init code 0x8C
            STwr::increaseSequenceNumber(),
            0,0, // PAN ID
            0,0,0,0,0,0,0,0,  // Tag address
            0,0, // Source address
            FN_RANGING_INITIATION, // Range Init Code
            0,0 // Tag Short Address
        };

        DW1000Ng::getNetworkId(&RangingInitiation[3]);
        memcpy(&RangingInitiation[5], tag_address_start, 8);
        RangingInitiation[5] = destination;
        DW1000Ng::getDeviceAddress(&RangingInitiation[13]);
        memcpy(&RangingInitiation[16], tag_address_start, 2);
        RangingInitiation[16] = destination;
        DW1000Ng::setTransmitData(RangingInitiation, sizeof(RangingInitiation));
        DW1000Ng::startTransmit();
        STwr::waitForTransmission();
    }

    void transmitResponseToPoll(byte destination) {
        byte poll_ack[] = {
            DATA,
            SHORT_SRC_AND_DEST,
            STwr::increaseSequenceNumber(),
            0,0, // PAN ID
            0,0, // TAG Short address
            0,0, // Source Address
            FN_RESPONSE, // Response Message ID
            0,0,0,0 // calculated TOF ? TODO
        };
        DW1000Ng::getNetworkId(&poll_ack[3]);
        memcpy(&poll_ack[5], tag_address_start, 2);
        poll_ack[5] = destination;
        DW1000Ng::getDeviceAddress(&poll_ack[7]);
        DW1000Ng::setTransmitData(poll_ack, sizeof(poll_ack));

        // printMessage(14, poll_ack);
        DW1000Ng::startTransmit();
        STwr::waitForTransmission();
    }

}