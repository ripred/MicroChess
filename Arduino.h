#ifndef ARDUINO_H
#define ARDUINO_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define PROGMEM
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#define pgm_read_dword(addr) (*(const uint32_t*)(addr))
#define pgm_get_far_address(addr) ((uintptr_t)(addr))
#define HIGH 1
#define LOW 0
#define INPUT 0
#define OUTPUT 1
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define PORTB (*((volatile uint8_t*)0))
#define PORTD (*((volatile uint8_t*)0))
#define ESP32 1
#define __malloc_heap_start ((char*)0)
inline long random(long max) { return rand() % max; }
inline long random(long min, long max) { return min + rand() % (max - min); }
inline char* dtostrf(double val, signed char width, unsigned char prec, char* buf) { sprintf(buf, "%*.*f", width, prec, val); return buf; }
inline void strcpy_P(char* dest, const char* src) { strcpy(dest, src); }
class HardwareSerial {
public:
    int available() { return 0; }
    int availableForWrite() { return 0; }
    int read() { return 0; }
    size_t write(uint8_t) { return 1; }
    size_t write(const char* s) { return printf("%s", s); }
    size_t write(const char* s, size_t) { return printf("%s", s); }
    void begin(long) {}
    operator bool() const { return true; }
};
static HardwareSerial Serial;
inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}
inline void delay(int) {}
inline uint32_t millis() { static uint32_t t = 0; return t += 16; }
#endif // ARDUINO_H
