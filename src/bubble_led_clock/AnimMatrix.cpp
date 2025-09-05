// AnimMatrix.cpp

#include "AnimMatrix.h"
#include <Arduino.h>

// --- Segment Definitions (Bit positions for a 7-segment display) ---
#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000

MatrixAnimation::MatrixAnimation(std::string targetText, unsigned long revealDuration, unsigned long holdTime, float rainSpeed)
    : _targetText(targetText),
      _revealDuration(revealDuration),
      _holdTime(holdTime),
      _rainSpeed(rainSpeed),
      _startTime(0),
      _holdPhaseStarted(false) {}

void MatrixAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    _startTime = millis();
    int size = _display->getDisplaySize();

    // Resize all our state vectors to match the display size
    _targetSegments.resize(size, 0);
    _revealedSegments.resize(size, 0);
    _rainPos.resize(size, std::vector<float>(3, 0.0f));

    // Pre-calculate the target segment masks for the final text
    int targetLength = _targetText.length();
    for (int i = 0; i < size; ++i) {
        int targetIndex = i - (size - targetLength);
        if (targetIndex >= 0 && targetIndex < targetLength) {
            _targetSegments[i] = getSegmentsForChar(_targetText[targetIndex]);
        }
    }

    // Initialize rain with random starting positions for a more natural look
    for (int i = 0; i < size; ++i) {
        _rainPos[i][0] = random(0, 30) / 10.0f; // Left column (F, E)
        _rainPos[i][1] = random(0, 40) / 10.0f; // Middle column (A, G, D)
        _rainPos[i][2] = random(0, 30) / 10.0f; // Right column (B, C)
    }
}

bool MatrixAnimation::isDone() {
    unsigned long currentTime = millis();
    // Animation is done if the reveal and hold times have both passed
    if (currentTime - _startTime > _revealDuration) {
        return (currentTime - (_startTime + _revealDuration)) >= _holdTime;
    }
    return false;
}

void MatrixAnimation::update() {
    if (isDone()) {
        return;
    }

    unsigned long currentTime = millis();
    int displaySize = _display->getDisplaySize();

    // --- REVEAL PHASE ---
    if (currentTime - _startTime < _revealDuration) {
        _display->clear();
        for (int i = 0; i < displaySize; ++i) {
            uint8_t fallingMask = 0;

            // Update and calculate falling segments for 3 vertical columns
            // Column 0: Left side (F -> E)
            _rainPos[i][0] += _rainSpeed;
            if (_rainPos[i][0] >= 2.0f) _rainPos[i][0] -= 2.0f;
            if ((int)_rainPos[i][0] == 0) fallingMask |= SEG_F;
            else fallingMask |= SEG_E;

            // Column 1: Middle (A -> G -> D)
            _rainPos[i][1] += _rainSpeed * 0.8f; // Vary speed slightly
            if (_rainPos[i][1] >= 3.0f) _rainPos[i][1] -= 3.0f;
            if ((int)_rainPos[i][1] == 0) fallingMask |= SEG_A;
            else if ((int)_rainPos[i][1] == 1) fallingMask |= SEG_G;
            else fallingMask |= SEG_D;

            // Column 2: Right side (B -> C)
            _rainPos[i][2] += _rainSpeed * 1.2f; // Vary speed slightly
            if (_rainPos[i][2] >= 2.0f) _rainPos[i][2] -= 2.0f;
            if ((int)_rainPos[i][2] == 0) fallingMask |= SEG_B;
            else fallingMask |= SEG_C;

            // "Lock in" segments that are part of the final text
            _revealedSegments[i] |= (fallingMask & _targetSegments[i]);

            // Combine falling and revealed segments and write to the buffer
            uint16_t finalMask = fallingMask | _revealedSegments[i];
            // This assumes your driver uses a buffer. If it writes directly,
            // you'd call setSegments(i, finalMask) or similar.
            _display->setSegments(i, finalMask);
        }
    } 
    // --- HOLD PHASE ---
    else {
        // Show the final, fully revealed text
        if (!_holdPhaseStarted) {
            _display->clear();
            for (int i = 0; i < displaySize; ++i) {
                 _display->setSegments(i, _targetSegments[i]);
            }
            _holdPhaseStarted = true;
        }
    }

    _display->writeDisplay();
}

// Minimal helper to convert char to segments.
// For a full implementation, use the font table from your driver.
uint8_t MatrixAnimation::getSegmentsForChar(char c) {
    // This is a simplified map. For a full character set, you should
    // use the PROGMEM font table from your HT16K33 driver.
    switch (c) {
        case '0': return 0b00111111;
        case '1': return 0b00000110;
        case '2': return 0b01011011;
        case '3': return 0b01001111;
        case '4': return 0b01100110;
        case '5': return 0b01101101;
        case '6': return 0b01111101;
        case '7': return 0b00000111;
        case '8': return 0b01111111;
        case '9': return 0b01101111;
        case ' ':
        default:  return 0b00000000;
    }
}