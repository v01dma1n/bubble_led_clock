#ifndef I_DISP_DRIVER_H
#define I_DISP_DRIVER_H

#include <Arduino.h> 

class IDisplayDriver {
public:
    // Virtual destructor is important for base classes
    virtual ~IDisplayDriver() {}

    // Initializes the display hardware
    virtual void begin() = 0;

    // Returns the number of characters the display can show
    virtual int getDisplaySize() = 0;

    // Sets the brightness (e.g., 0-15)
    virtual void setBrightness(uint8_t level) = 0;

    // Clears the entire display
    virtual void clear() = 0;

    // Sets a single character at a specific position in the buffer
    // The `dot` parameter controls the decimal point.
    virtual void setChar(int position, char character, bool dot = false) = 0;

    virtual void setSegments(int position, uint8_t mask) = 0;

    void print(const char* text) {
        clear();

        // Loop through the input text and the display positions
        // Stop when we either run out of display space or the string ends
        int displaySize = getDisplaySize();
        for (int i = 0; i < displaySize && text[i] != '\0'; ++i) {
            setChar(i, text[i], false);
        }
        writeDisplay();
    }

    // Pushes the character buffer to the physical display to make it visible
    virtual void writeDisplay() = 0;
};

#endif // I_DISP_DRIVER_H