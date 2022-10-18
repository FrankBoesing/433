#pragma once
#include <Arduino.h>

#define rcv433_pin D2 // The pin connected to the receiver

struct tempData433_t
{
  const char *protocolname;
  float temp;
  uint8_t id;
  uint8_t channel;
  uint8_t battery;
  uint8_t button;
  uint8_t humidity;
} __attribute__((packed));


void init433();
bool read433(tempData433_t *td);
