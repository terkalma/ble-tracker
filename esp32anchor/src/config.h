#ifndef CONFIG_H
#define CONFIG_H

#include "Arduino.h"

// leftmost two bytes below will become the "short address"
char anchor_addr[23]{'8', '0', ':', '0', '0', ':', '5', 'B', ':', 'D', '5', ':', 'A', '9', ':', '9', 'A', ':', 'E', '2', ':', '9', 'C'};

#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4

// connection pins
const uint8_t PIN_RST = 27; // reset pin
const uint8_t PIN_IRQ = 34; // irq pin
const uint8_t PIN_SS = 4;   // spi select pin

#endif