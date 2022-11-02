#ifndef CONFIG_H
#define CONFIG_H
#include "Arduino.h"
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin
IPAddress server(192, 168, 0, 234);

char tag_addr[23] = {'7', '0', ':', '0', '0', ':', '2', '2', ':', 'E', 'A', ':', '8', '2', ':', '6', '0', ':', '3', 'B', ':', '9', 'C'};

#define WIFI_UN "UPC5501473"
#define WIFI_PW "d7mvswshZddh"
#define UNAME "tag"
#define PWD "potato666"

#define ANCHOR_COUNT 2

#endif