#ifndef DISP_DRIVER_HT16K33_H
#define DISP_DRIVER_HT16K33_H

#include <Arduino.h> 
#include "IDispDriver.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

class DispDriverHT16K33 : public IDisplayDriver {
public:
    DispDriverHT16K33(uint8_t i2c_addr = 0x70, int displaySize = 8);
    virtual ~DispDriverHT16K33() {}

    void begin() override;
    int getDisplaySize() override;
    void setBrightness(uint8_t level) override;

    void clear() override;
    void setChar(int position, char character, bool dot = false) override;
    void setSegments(int position, uint8_t mask) override;

    void writeDisplay() override;

private:
    Adafruit_7segment _display;
    int _displaySize;
    uint8_t _i2c_addr;

    // Font table for character-to-segment mapping
    static const uint8_t PROGMEM sevensegfonttable[]; 
};

#endif // DISP_DRIVER_HT16K33_H
