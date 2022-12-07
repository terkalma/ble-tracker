#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

#define WIFI_UN "_LinksysSetup5D0"
#define WIFI_PW "svd50ximea"

#define REDIS_ADDR "192.168.1.58"
#define REDIS_PORT 6379
#define REDIS_PASSWORD "potato666"

#define P(...) Serial.print(__VA_ARGS__)
#define PLN(...) Serial.println(__VA_ARGS__)

#ifdef DEBUG
  #define DP(...) Serial.print(__VA_ARGS__)
  #define DPLN(...) Serial.println(__VA_ARGS__)
#else
  #define DP(...)
  #define DPLN(...)
#endif

#endif