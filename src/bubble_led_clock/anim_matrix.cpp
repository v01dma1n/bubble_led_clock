#include <Arduino.h>
#include "anim_matrix.h"

#define SEG_A 0b00000001
#define SEG_B 0b00000010
#define SEG_C 0b00000100
#define SEG_D 0b00001000
#define SEG_E 0b00010000
#define SEG_F 0b00100000
#define SEG_G 0b01000000

MatrixAnimation::MatrixAnimation(std::string targetText, unsigned long revealDelay, unsigned long holdTime, unsigned long rainDelay, bool dotsWithPreviousChar)
    : _targetText(targetText),
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _revealDelay(revealDelay),
      _holdTime(holdTime),
      _rainDelay(rainDelay),
      _lastRainTime(0),
      _lastRevealTime(0),
      _revealCompleteTime(0),
      _revealedCount(0) {}

void MatrixAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    int size = _display->getDisplaySize();

    // --- Pre-parse the target string to handle dots ---
    _parsedTargetText = "";
    _dotState.assign(size, false);
    
    int text_idx = 0;
    int display_idx = 0;
    while (text_idx < _targetText.length() && display_idx < size) {
        char currentChar = _targetText[text_idx];
        if (currentChar == '.' && _dotsWithPreviousChar) {
            if (display_idx > 0) {
                _dotState[display_idx - 1] = true;
            }
            text_idx++;
            continue;
        }
        _parsedTargetText += currentChar;
        text_idx++;
        display_idx++;
    }
    // Pad with spaces if needed
    while (_parsedTargetText.length() < size) {
        _parsedTargetText += ' ';
    }

    // Initialize rain positions
    _rainPos.resize(size, std::vector<float>(3, 0.0f));
    for (int i = 0; i < size; ++i) {
        _rainPos[i][0] = random(0, 20) / 10.0f;
        _rainPos[i][1] = random(0, 30) / 10.0f;
        _rainPos[i][2] = random(0, 20) / 10.0f;
    }
}

bool MatrixAnimation::isDone() {
    bool revealComplete = _revealedCount >= _display->getDisplaySize();
    if (!revealComplete) {
        return false;
    }
    return (millis() - _revealCompleteTime >= _holdTime);
}

void MatrixAnimation::update() {
    unsigned long currentTime = millis();
    if (currentTime - _lastRainTime < _rainDelay) {
        return; // Control frame rate
    }
    _lastRainTime = currentTime;

    if (isDone()) {
        return;
    }

    int displaySize = _display->getDisplaySize();

    // --- Reveal Logic: Reveal one character at a time ---
    bool revealPhaseActive = _revealedCount < displaySize;
    if (revealPhaseActive && (currentTime - _lastRevealTime >= _revealDelay)) {
        _lastRevealTime = currentTime;
        _revealedCount++;
        if (_revealedCount == displaySize) {
            _revealCompleteTime = millis();
        }
    }

    _display->clear();
    for (int i = 0; i < displaySize; ++i) {
        if (i < _revealedCount) {
            // This character is revealed, show it from the parsed text
            _display->setChar(i, _parsedTargetText[i], _dotState[i]);
        } else {
            // This character is not yet revealed, show rain animation
            uint8_t fallingMask = 0;
            _rainPos[i][0] += 0.1f;
            if (_rainPos[i][0] >= 2.0f) _rainPos[i][0] -= 2.0f;
            if ((int)_rainPos[i][0] == 0) fallingMask |= SEG_F; else fallingMask |= SEG_E;

            _rainPos[i][1] += 0.08f;
            if (_rainPos[i][1] >= 3.0f) _rainPos[i][1] -= 3.0f;
            if ((int)_rainPos[i][1] == 0) fallingMask |= SEG_A;
            else if ((int)_rainPos[i][1] == 1) fallingMask |= SEG_G; else fallingMask |= SEG_D;

            _rainPos[i][2] += 0.12f;
            if (_rainPos[i][2] >= 2.0f) _rainPos[i][2] -= 2.0f;
            if ((int)_rainPos[i][2] == 0) fallingMask |= SEG_B; else fallingMask |= SEG_C;
            
            _display->setSegments(i, fallingMask);
        }
    }
    _display->writeDisplay();
}

