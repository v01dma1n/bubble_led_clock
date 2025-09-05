// DispDriverHT16K33.cpp

#include "DispDriverHT16K33.h"
#include <Arduino.h>
#include <cctype> // Add this line for toupper()

// --- Font Table ---
// This table maps ASCII characters to their 7-segment bit patterns.
// It's defined as 'static const' to be part of this file's compilation unit.
const uint8_t PROGMEM DispDriverHT16K33::sevensegfonttable[] = {
    0b00000000, // (space)
    0b10000110, // !
    0b00100010, // "
    0b01111110, // #
    0b01101101, // $
    0b11010010, // %
    0b01000110, // &
    0b00100000, // '
    0b00101001, // (
    0b00001011, // )
    0b00100001, // *
    0b01110000, // +
    0b00010000, // ,
    0b01000000, // -
    0b10000000, // .
    0b01010010, // /
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b00001001, // :
    0b00001101, // ;
    0b01100001, // <
    0b01001000, // =
    0b01000011, // >
    0b11010011, // ?
    0b01011111, // @
    0b01110111, // A
    0b01111100, // B or b
    0b00111001, // C
    0b01011110, // D or d
    0b01111001, // E
    0b01110001, // F
    0b00111101, // G
    0b01110110, // H
    0b00110000, // I
    0b00011110, // J
    0b01110101, // K
    0b00111000, // L
    0b00010101, // M
    0b00110111, // N
    0b00111111, // O
    0b01110011, // P
    0b01101011, // Q
    0b00110011, // R
    0b01101101, // S
    0b01111000, // T
    0b00111110, // U
    0b00111110, // V
    0b00101010, // W
    0b01110110, // X
    0b01101110, // Y
    0b01011011, // Z
};


DispDriverHT16K33::DispDriverHT16K33(uint8_t i2c_addr, int displaySize) 
    : _display(), _displaySize(displaySize), _i2c_addr(i2c_addr) {}

void DispDriverHT16K33::begin() {
    // Initialize the I2C connection with the specified address 
    _display.begin(_i2c_addr); 
}

int DispDriverHT16K33::getDisplaySize() {
    return _displaySize;
}

void DispDriverHT16K33::setBrightness(uint8_t level) {
    // Set the display brightness (0-15) 
    _display.setBrightness(level); 
}

void DispDriverHT16K33::clear() {
    // Clear the internal display buffer
    _display.clear(); 
}

void DispDriverHT16K33::setChar(int position, char character, bool dot) {
    // Validate position to prevent writing out of bounds
    if (position < 0 || position >= _displaySize) {
        return;
    }

    // Convert any lowercase character to its uppercase equivalent
    character = toupper(character);

    // Look up the character's segment pattern from the font table.
    // The logic `character - ' '` calculates the index in the table.
    uint8_t segments = pgm_read_byte(sevensegfonttable + character - ' '); 
    // If the dot flag is set, combine the segments with the decimal point bit
    if (dot) {
        segments |= 0b10000000;
    }

    // Place the final segment pattern into the display buffer.
    // The buffer is mapped with position 0 on the right, so we reverse it.
    _display.displaybuffer[_displaySize - 1 - position] = segments; 
}

void DispDriverHT16K33::setSegments(int position, uint8_t mask) {
    if (position < 0 || position >= _displaySize) {
        return;
    }
    // Place the raw mask into the display buffer, reversing for right-to-left
    _display.displaybuffer[_displaySize - 1 - position] = mask;
}

void DispDriverHT16K33::writeDisplay() {
    // Send the contents of the display buffer to the hardware driver 
    _display.writeDisplay(); 
}
