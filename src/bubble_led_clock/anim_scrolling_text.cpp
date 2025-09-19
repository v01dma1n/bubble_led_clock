#include "anim_scrolling_text.h"

#include <Arduino.h>

ScrollingTextAnimation::ScrollingTextAnimation(std::string text, unsigned long scrollDelay, bool dotsWithPreviousChar)
    : _text(text),
      _scrollDelay(scrollDelay),
      _lastScrollTime(0),
      _currentPosition(0),
      _dotsWithPreviousChar(dotsWithPreviousChar)  {}

void ScrollingTextAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);

    if (_dotsWithPreviousChar) {
        // Pre-parse the text to separate characters and dots
        for (size_t i = 0; i < _text.length(); ++i) {
            if (_text[i] == '.') {
                if (!_dotStates.empty()) {
                    _dotStates.back() = true; // Apply the dot to the previous character
                }
            } else {
                _parsedText += _text[i];
                _dotStates.push_back(false);
            }
        }
    } else {
        _parsedText = _text;
    }

    _currentPosition = -_display->getDisplaySize();
}

bool ScrollingTextAnimation::isDone() {
    return _currentPosition >= (int)_text.length();
}

void ScrollingTextAnimation::update() {
    if (isDone()) {
        return;
    }

    unsigned long currentTime = millis();
    if (currentTime - _lastScrollTime >= _scrollDelay) {
        _lastScrollTime = currentTime;

        _display->clear();
        int displaySize = _display->getDisplaySize();

        for (int i = 0; i < displaySize; ++i) {
            int textIndex = _currentPosition + i;
            if (textIndex >= 0 && textIndex < (int)_parsedText.length()) {
                bool hasDot = _dotsWithPreviousChar ? _dotStates[textIndex] : false;
                _display->setChar(i, _parsedText[textIndex], hasDot);
            }
        }

        _display->writeDisplay();
        _currentPosition++;
    }
}
