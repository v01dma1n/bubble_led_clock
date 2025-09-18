// File: disp_driver_ht16k33.cpp

#include "disp_driver_ht16k33.h"
#include <Arduino.h>
#include <cctype> 
#include <Wire.h> // Use the standard I2C library

// HT16K33 Command Constants from datasheet
#define HT16K33_CMD_OSCILLATOR_ON  0x21
#define HT16K33_CMD_DISPLAY_ON     0x81
#define HT16K33_CMD_BRIGHTNESS     0xE0

// Font Table (this is unchanged)
const uint8_t PROGMEM DispDriverHT16K33::sevensegfonttable[] = {
    // ... font data remains here, no changes needed ...
    0b00000000, 0b10000110, 0b00100010, 0b01111110, 0b01101101, 0b11010010,
    0b01000110, 0b00100000, 0b00101001, 0b00001011, 0b00100001, 0b01110000,
    0b00010000, 0b01000000, 0b10000000, 0b01010010, 0b00111111, 0b00000110,
    0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111,
    0b01111111, 0b01101111, 0b00001001, 0b00001101, 0b01100001, 0b01001000,
    0b01000011, 0b11010011, 0b01011111, 0b01110111, 0b01111100, 0b00111001,
    0b01011110, 0b01111001, 0b01110001, 0b00111101, 0b01110110, 0b00110000,
    0b00011110, 0b01110101, 0b00111000, 0b00010101, 0b00110111, 0b00111111,
    0b01110011, 0b01101011, 0b00110011, 0b01101101, 0b01111000, 0b00111110,
    0b00111110, 0b00101010, 0b01110110, 0b01101110, 0b01011011
};

DispDriverHT16K33::DispDriverHT16K33(uint8_t i2c_addr, int displaySize) 
    : _displaySize(displaySize), _i2c_addr(i2c_addr) {
    // Initialize buffer
    for (int i = 0; i < 8; i++) {
        _displayBuffer[i] = 0;
    }
}

void DispDriverHT16K33::begin() {
    // Turn on the oscillator
    Wire.beginTransmission(_i2c_addr);
    Wire.write(HT16K33_CMD_OSCILLATOR_ON);
    Wire.endTransmission();
    // Turn on the display, no blinking
    Wire.beginTransmission(_i2c_addr);
    Wire.write(HT16K33_CMD_DISPLAY_ON);
    Wire.endTransmission();
    // Set max brightness
    setBrightness(15);
}

int DispDriverHT16K33::getDisplaySize() {
    return _displaySize;
}

void DispDriverHT16K33::setBrightness(uint8_t level) {
    if (level > 15) level = 15; // Brightness is 0-15
    Wire.beginTransmission(_i2c_addr);
    Wire.write(HT16K33_CMD_BRIGHTNESS | level);
    Wire.endTransmission();
}

void DispDriverHT16K33::clear() {
    for (int i = 0; i < 8; i++) {
        _displayBuffer[i] = 0;
    }
}

void DispDriverHT16K33::writeDisplay() {
    Wire.beginTransmission(_i2c_addr);
    Wire.write((uint8_t)0x00); // Start at RAM address 0

    // Write all 8 digit values (16 bytes) to the display RAM
    for (uint8_t i = 0; i < 8; i++) {
        Wire.write(_displayBuffer[i] & 0xFF);
        Wire.write(_displayBuffer[i] >> 8);
    }
    Wire.endTransmission();
}

void DispDriverHT16K33::setChar(int position, char character, bool dot) {
    if (position < 0 || position >= _displaySize) return;

    character = toupper(character);
    uint8_t segments = pgm_read_byte(sevensegfonttable + character - ' ');
    if (dot) {
        segments |= 0b10000000;
    }
    // The Adafruit library maps digits right-to-left, so we preserve that
    _displayBuffer[_displaySize - 1 - position] = segments;
}

void DispDriverHT16K33::setSegments(int position, uint8_t mask) {
    if (position < 0 || position >= _displaySize) return;
    _displayBuffer[_displaySize - 1 - position] = mask;
}

