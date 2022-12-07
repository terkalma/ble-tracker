#include <DW1000Ng.hpp>
#include "STwr.h"
#include "config.h"

char EUI[] = "70:AA:BB:CC:DD:EE:01:00";

#define FACTORY_ANTENNA_DELAY 16384

volatile uint32_t blink_rate = 200;

device_configuration_t DEFAULT_CONFIG = {
    false,
    true,
    true,
    true,
    false,
    SFDMode::STANDARD_SFD,
    Channel::CHANNEL_5,
    DataRate::RATE_850KBPS,
    PulseFrequency::FREQ_16MHZ,
    PreambleLength::LEN_256,
    PreambleCode::CODE_3
};

frame_filtering_configuration_t TAG_FRAME_FILTER_CONFIG = {
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    false
};

sleep_configuration_t SLEEP_CONFIG = {
    false,  // onWakeUpRunADC   reg 0x2C:00
    false,  // onWakeUpReceive
    false,  // onWakeUpLoadEUI
    true,   // onWakeUpLoadL64Param
    true,   // preserveSleep
    true,   // enableSLP    reg 0x2C:06
    false,  // enableWakePIN
    true    // enableWakeSPI
};

void setup() {
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println(F("### DW1000Ng-arduino-ranging-tag ###"));
    // initialize the driver

    EUI[22] = '0' + TAG_INDEX;
    DW1000Ng::initializeNoInterrupt(PIN_SS, PIN_RST);
    Serial.println("DW1000Ng initialized ...");
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(TAG_FRAME_FILTER_CONFIG);

    DW1000Ng::setEUI(EUI);
    DW1000Ng::setDeviceAddress(TAG_INDEX + 256);
    DW1000Ng::setNetworkId(RTLS_APP_ID);

    DW1000Ng::setAntennaDelay(FACTORY_ANTENNA_DELAY); // Tags are set to factory default

    DW1000Ng::applySleepConfiguration(SLEEP_CONFIG);

    DW1000Ng::setPreambleDetectionTimeout(15);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(2000);

    Serial.println(F("Committed configuration ..."));
    // DEBUG chip info and registers pretty printed
    char msg[128];
    DW1000Ng::getPrintableDeviceIdentifier(msg);
    Serial.print("Device ID: "); Serial.println(msg);
    DW1000Ng::getPrintableExtendedUniqueIdentifier(msg);
    Serial.print("Unique ID: "); Serial.println(msg);
    DW1000Ng::getPrintableNetworkIdAndShortAddress(msg);
    Serial.print("Network ID & Device Address: "); Serial.println(msg);
    DW1000Ng::getPrintableDeviceMode(msg);
    Serial.print("Device mode: "); Serial.println(msg);
}

void loop() {
    if (STwr::receiveFrame()) {
        size_t len = DW1000Ng::getReceivedDataLength();
        byte data[len];
        DW1000Ng::getReceivedData(data, len);
        if(len > 15 && data[15] == FN_RANGING_INITIATION) {
            STwr::transmitPoll(data[13]);
            if (STwr::receiveFrame()) {
                size_t len = DW1000Ng::getReceivedDataLength();
                byte data[len];
                DW1000Ng::getReceivedData(data, len);
                // Check if it's a poll ACK
                if (len > 9 && data[9] == FN_RESPONSE) {
                    STwr::transmitFinal(data[7]);
                } else {
                    Serial.println("Error in PR");
                    STwr::printMessage(len, data);
                }
            } else {
                Serial.println("Did not receive a PR");
            }
        } else {
            Serial.println("Error in RI");
            STwr::printMessage(len, data);
        }
    }
}
