#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgRanging.hpp>
#include "STwr.h"
#include "config.h"
#include "WiFi.h"
#include <WiFiClient.h>
#include <Redis.h>

WiFiClient redisConn;
Redis* redis_client;


double measureDistance(byte tag_id)
{
  STwr::transmitRangeInit(tag_id);
  if (STwr::receiveFrame()) {
    size_t len = DW1000Ng::getReceivedDataLength();
    byte data[len];
    DW1000Ng::getReceivedData(data, len);
    if (len > 9 && data[9] == FN_POLL) {
      uint64_t time_poll_received = DW1000Ng::getReceiveTimestamp();
      STwr::transmitResponseToPoll(tag_id);
      uint64_t time_response_to_poll = DW1000Ng::getTransmitTimestamp();
      delayMicroseconds(1300); // final response from anchor is delayed by 1500
      if (STwr::receiveFrame()) {
        size_t len = DW1000Ng::getReceivedDataLength();
        byte data[len];
        DW1000Ng::getReceivedData(data, len);
        if (len > 18 && data[9] == FN_FINAL_RESPONSE) {
          uint64_t time_final_message = DW1000Ng::getReceiveTimestamp();
          double range = DW1000NgRanging::computeRangeAsymmetric(
              DW1000NgUtils::bytesAsValue(data + 10, LENGTH_TIMESTAMP), // Poll send time
              time_poll_received,
              time_response_to_poll, // Response to poll sent time
              DW1000NgUtils::bytesAsValue(data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
              DW1000NgUtils::bytesAsValue(data + 18, LENGTH_TIMESTAMP), // Final Message send time
              time_final_message // Final message receive time
          );
          range = DW1000NgRanging::correctRange(range);
          if (range < 0) {
            return 0.001;
          }
          return range;
        } else {
          Serial.println("Not an RF message");
          STwr::printMessage(len, data);
        }
      } else {
        Serial.println("Did not receive an RF message");
      }
    } else {
      Serial.println("Data out of sync");
      STwr::printMessage(len, data);
    }
  }

  return -1.0;
}


double measureRobust(byte tag_id, byte count)
{
  byte successes = 0;
  double d;
  double final_d = 0.0;
  for (byte i=0; i<count; i++) {
    d = measureDistance(tag_id);

    if (d >= 0) {
      successes += 1;
      final_d += d;
    }
  }

  if (successes > 0) {
    return final_d / successes;
  } else {
    return -1.0;
  }
}


// void callback(char* topic, byte* payload, unsigned int length) {
//   byte tag_index = payload[0] - '0';

//   if (tag_index > 0 && tag_index < 10) {
//     String msg = String(ANCHOR_INDEX) + String(":") + String(tag_index) + String(":");
//     msg += String(measureRobust(tag_index, 10));
//     Serial.print("Publishing: "); Serial.println(msg);
//     client.publish("measurement", msg.c_str());
//   }
// }

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_UN, WIFI_PW);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

// Extended Unique Identifier register. 64-bit device identifier.
char EUI[] = "80:BB:CC:DD:EE:FF:00:00"; // ANCHOR INDEX will added to the last bit.

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

frame_filtering_configuration_t ANCHOR_FRAME_FILTER_CONFIG = {
    false,
    false,
    true,
    false,
    false,
    false,
    false,
    true /* This allows blink frames */
};

void setup() {
    EUI[22] = '0' + ANCHOR_INDEX;
    // DEBUG monitoring
    Serial.begin(115200);
    Serial.println("### DW1000Ng-ranging-anchor " + String(ANCHOR_INDEX) + " ###");
    // initialize the driver
    DW1000Ng::initializeNoInterrupt(PIN_SS, PIN_RST);
    Serial.println(F("DW1000Ng initialized ..."));
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);
    DW1000Ng::setEUI(EUI);
    DW1000Ng::setPreambleDetectionTimeout(64);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(5000);
    DW1000Ng::setNetworkId(RTLS_APP_ID);
    DW1000Ng::setDeviceAddress(ANCHOR_INDEX);

    // TODO calibrate
    DW1000Ng::setAntennaDelay(16390);

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

    connectToWifi();
    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))
    {
        Serial.println("Failed to connect to the Redis server!");
        return;
    }
    redis_client = new Redis(redisConn);
    auto connRet = redis_client->authenticate(REDIS_PASSWORD);
    if (connRet == RedisSuccess)
    {
        Serial.println("Connected to the Redis server!");
    }
    else
    {
        Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet);
        return;
    }
}

void loop() {
  if (redis_client != NULL) {
    Serial.println(redis_client->get((String("anchor") + String(ANCHOR_INDEX)).c_str()));
  }

  delay(10);
}