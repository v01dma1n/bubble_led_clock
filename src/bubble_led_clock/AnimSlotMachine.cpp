// AnimSlotMachine.cpp

#define ESP32DEBUGGING
#include <ESP32Logger.h>
#include <Arduino.h>

#include "AnimSlotMachine.h"

SlotMachineAnimation::SlotMachineAnimation(std::string targetText, unsigned long lockDelay, unsigned long holdTime, unsigned long spinDelay)
    : _targetText(targetText),
      _lockDelay(lockDelay),
      _holdTime(holdTime),
      _spinDelay(spinDelay), // Initialize spinDelay
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
    int targetLength = _targetText.length();

    _display->clear();
    for (int i = 0; i < displaySize; ++i) {
        int targetIndex = i - (displaySize - targetLength);
        char charToWrite;

        if (_isLocked[i]) {
            charToWrite = (targetIndex >= 0 && targetIndex < targetLength) ? _targetText[targetIndex] : ' ';
        } else {
            charToWrite = random(0, 'Z'-'0') + '0';  // range of all numeric and non-numeric characters
        }

        // disable the logic of using a decimal point"
        // if (charToWrite == '.') {
        //     int prevIndex = i - 1;
        //     if(prevIndex >= 0) {
        //         char prevChar = (targetIndex > 0) ? _targetText[targetIndex-1] : ' ';
        //         _display->setChar(prevIndex, prevChar, true);
        //     }
        //     continue;
        // }
        _display->setChar(i, charToWrite, false);
    }
    _display->writeDisplay();
}