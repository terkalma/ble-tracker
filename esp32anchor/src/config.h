#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

IPAddress server(192, 168, 0, 234);

#define WIFI_UN "UPC5501473"
#define WIFI_PW "d7mvswshZddh"
#define UNAME "tag"
#define PWD "potato666"

#endif