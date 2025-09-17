#include "anim_slot_machine.h"

#include <Arduino.h>


SlotMachineAnimation::SlotMachineAnimation(std::string targetText, 
                                           unsigned long lockDelay, 
                                           unsigned long holdTime, 
                                           unsigned long spinDelay, 
                                           bool dotsWithPreviousChar)
    : _targetText(targetText),
      _lockDelay(lockDelay),
      _holdTime(holdTime),
      _spinDelay(spinDelay), // Initialize spinDelay
      _dotsWithPreviousChar(dotsWithPreviousChar),
      _lastLockTime(0),
      _lastSpinTime(0),      // Initialize spin timer
      _lockedCount(0),
      _lockingCompleteTime(0) {
    _isLocked = nullptr;
}

SlotMachineAnimation::~SlotMachineAnimation() {
    delete[] _isLocked;
}

void SlotMachineAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
    int size = _display->getDisplaySize();
    _currentText.resize(size, ' ');

    delete[] _isLocked;
    _isLocked = new bool[size];
    for (int i = 0; i < size; ++i) {
        _isLocked[i] = false;
    }
    
    // Add this line to start the timer correctly
    _lastLockTime = millis();
    
    randomSeed(analogRead(0));
}

bool SlotMachineAnimation::isDone() {
    bool lockingComplete = _lockedCount >= _display->getDisplaySize();
    // if (!lockingComplete) {
    //     return false;
    // }
    // return (millis() - _lockingCompleteTime >= _holdTime);
    return lockingComplete;
}

void SlotMachineAnimation::update() {
    unsigned long currentTime = millis();

    // Only update the display if spinDelay has passed
    if (currentTime - _lastSpinTime < _spinDelay) {
        return; // Not time to update yet
    }
    _lastSpinTime = currentTime; // Record the time of this update

    if (isDone()) {
        return;
    }

    // --- Logic for locking characters ---
    bool lockingPhaseActive = _lockedCount < _display->getDisplaySize();
    if (lockingPhaseActive) {
        if (currentTime - _lastLockTime >= _lockDelay) {
            _lastLockTime = currentTime;
            if (_lockedCount < _display->getDisplaySize()) {
                _isLocked[_lockedCount] = true;
                _lockedCount++;

                if (_lockedCount == _display->getDisplaySize()) {
                    _lockingCompleteTime = millis();
                }
            }
        }
    }

    // --- Display Drawing Logic ---
     int displaySize = _display->getDisplaySize();
    _display->clear();

    // Use two different drawing loops based on the dot handling mode
    if (_dotsWithPreviousChar) {
        int text_idx = 0;
        int display_idx = 0;
        while (text_idx < _targetText.length() && display_idx < displaySize) {
            char charToProcess = _targetText[text_idx];
            if (charToProcess == '.') {
                text_idx++;
                continue;
            }
            bool hasDot = (_targetText[text_idx + 1] == '.');
            char charToWrite = _isLocked[display_idx] ? charToProcess : (random(0, 10) + '0');
            _display->setChar(display_idx, charToWrite, hasDot);
            text_idx++;
            display_idx++;
        }
    } else {
        for (int i = 0; i < displaySize; ++i) {
            char charToProcess = (i < _targetText.length()) ? _targetText[i] : ' ';
            char charToWrite = _isLocked[i] ? charToProcess : (random(0, 10) + '0');
            _display->setChar(i, charToWrite, false);
        }
    }
    _display->writeDisplay();
}