#include <DW1000Ng.hpp>
#include <DW1000NgUtils.hpp>
#include <DW1000NgRanging.hpp>
#include "STwr.h"
#include "config.h"
#include "WiFi.h"
#include <WiFiClient.h>
#include <Redis.h>
#include <esp_task_wdt.h>


long delays[4] = {
  16485,
  16485,
  16500,
  16510};


#define WDT_TIMEOUT 5

// long success = 0;
// long fail = 0;

WiFiClient redisConn;
Redis* redis_client;

void measureDistance(byte tag_id, double* result)
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
          result[0] = DW1000NgRanging::computeRangeAsymmetric(
              DW1000NgUtils::bytesAsValue(data + 10, LENGTH_TIMESTAMP), // Poll send time
              time_poll_received,
              time_response_to_poll, // Response to poll sent time
              DW1000NgUtils::bytesAsValue(data + 14, LENGTH_TIMESTAMP), // Response to Poll Received
              DW1000NgUtils::bytesAsValue(data + 18, LENGTH_TIMESTAMP), // Final Message send time
              time_final_message // Final message receive time
          );
          result[1] = DW1000NgRanging::correctRange(result[0]);

          for (byte i = 0; i < 2; i++) {
            if (result[i] < 0) {
              result[i] = 0.001;
            }
          }

          result[2] = -(static_cast<double>(DW1000Ng::getReceivePower()));
          return;
        } else {
          PLN("Not an RF message");
          STwr::printMessage(len, data);
        }
      } else {
        PLN("Did not receive an RF message");
      }
    } else {
      PLN("Data out of sync");
      STwr::printMessage(len, data);
    }
  }

  result[0] = -1.0;
}


void measureRobust(byte tag_id, byte count, double* final_result)
{
  byte successes = 0;
  double final_d = 0.0;
  double final_power = 0.0;
  double final_uncorrected = 0.0;
  for (byte i=0; i<count; i++) {
    double result[3] = {0.0, 0.0, 0.0};
    measureDistance(tag_id, &result[0]);
    // delay(1);
    if (result[0] >= 0) {
      successes += 1;
      final_result[0] += result[0];
      final_result[1] += result[1];
      final_result[2] += result[2];
    }
  }

  if (successes > 0) {
    final_result[0] /= successes;
    final_result[1] /= successes;
    final_result[2] /= successes;
  } else {
    final_result[0] = -1.0;
  }
}

void connectToWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_UN, WIFI_PW);

  long t = millis();
  while (WiFi.status() != WL_CONNECTED) {

    if (millis() - t > 10000) {
      ESP.restart();
    }
    delay(500);
    P(".");
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
    PLN("### DW1000Ng-ranging-anchor " + String(ANCHOR_INDEX) + " ###");
    // initialize the driver
    DW1000Ng::initializeNoInterrupt(PIN_SS, PIN_RST);
    PLN("DW1000Ng initialized ...");
    // general configuration
    DW1000Ng::applyConfiguration(DEFAULT_CONFIG);
    DW1000Ng::enableFrameFiltering(ANCHOR_FRAME_FILTER_CONFIG);
    DW1000Ng::setEUI(EUI);
    DW1000Ng::setPreambleDetectionTimeout(64);
    DW1000Ng::setSfdDetectionTimeout(273);
    DW1000Ng::setReceiveFrameWaitTimeoutPeriod(5000);
    DW1000Ng::setNetworkId(RTLS_APP_ID);
    DW1000Ng::setDeviceAddress(ANCHOR_INDEX);

    // Maybe this should be different for each tag??
    DW1000Ng::setAntennaDelay(delays[ANCHOR_INDEX-1]);

    PLN("Committed configuration ...");
    // DEBUG chip info and registers pretty printed
    char msg[128];
    DW1000Ng::getPrintableDeviceIdentifier(msg);
    P("Device ID: "); PLN(msg);
    DW1000Ng::getPrintableExtendedUniqueIdentifier(msg);
    P("Unique ID: "); PLN(msg);
    DW1000Ng::getPrintableNetworkIdAndShortAddress(msg);
    Serial.print("Network ID & Device Address: "); PLN(msg);
    DW1000Ng::getPrintableDeviceMode(msg);
    Serial.print("Device mode: "); PLN(msg);

    connectToWifi();
    if (!redisConn.connect(REDIS_ADDR, REDIS_PORT))
    {
        PLN("Failed to connect to the Redis server!");
        return;
    }
    redis_client = new Redis(redisConn);
    auto connRet = redis_client->authenticate(REDIS_PASSWORD);
    if (connRet == RedisSuccess)
    {
        PLN("Connected to the Redis server!");
    }
    else
    {
        P("Failed to authenticate to the Redis server! Errno: ");
        PLN((int)connRet);
        delay(1000);
        ESP.restart();
    }

    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
}

String ping_key = String("anchor") + String(ANCHOR_INDEX) + String("ping");
String get_key = String("anchor") + String(ANCHOR_INDEX);
String set_key = String("anchor") + String(ANCHOR_INDEX) + String("r");

void loop() {
  // Restart ESP if WIFI is not connected
  if ((WiFi.status() != WL_CONNECTED)) {
    delay(1000);
    ESP.restart();
  }

  if (redis_client != NULL) {
    // long t = millis();
    String result = redis_client->get(get_key.c_str());
    // Serial.println(millis() - t);
    if (result.length() > 0) {
      byte tag_index = result.charAt(0) - '0';
      if (tag_index > 0) {
        double result[3] = {0.0, 0.0, 0.0};
        measureDistance(tag_index, &result[0]);
        String report = String(tag_index);
        for (byte i=0; i<3; i++) {
          report += String(":") + String(result[i]);
        }
        esp_task_wdt_reset();
        redis_client->del(get_key.c_str());
        redis_client->set(set_key.c_str(), report.c_str());
        esp_task_wdt_reset();
      }
    }
  } else {
    delay(1000);
    ESP.restart(); // restart ESP if redis wasn't propertly instantiated
  }

  esp_task_wdt_reset();
  delay(50);
}