#pragma once
#include <Arduino.h>

//TODO: Add digitalReadFast for more boards
#if defined(ARDUINO_ARCH_ESP8266)
#define GPIO_IN ((volatile uint32_t *)0x60000318)
#define digitalReadFast(pin) ( ((*GPIO_IN & (1 << (pin)))) ? 1 : 0)
#define RAMFUNC IRAM_ATTR

#elif defined(ARDUINO_ARCH_ESP32)

#if CONFIG_IDF_TARGET_ESP32C3
#define digitalReadFast(pin) ( (GPIO.in.val & (1 << (pin))) ? 1 : 0)
#else // plain ESP32
#define digitalReadFast(pin) ( ((pin) < 32) ? (GPIO.in & (1 << (pin))) : (GPIO.in1.val & (1 << ((pin) - 32)) ) ? 1 : 0 )
#endif
#define RAMFUNC IRAM_ATTR

#elif defined(CORE_TEENSY)
//digitalReadFast() exists
#define RAMFUNC FASTRUN

#else // unknown
#define digitalReadFast(pin) (digitalRead(pin) ? 1 : 0)
#define RAMFUNC
#endif
