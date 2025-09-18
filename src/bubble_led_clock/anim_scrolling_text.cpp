#include "anim_scrolling_text.h"

#include <Arduino.h>

ScrollingTextAnimation::ScrollingTextAnimation(std::string text, unsigned long scrollDelay)
    : _text(text),
      _scrollDelay(scrollDelay),
      _lastScrollTime(0),
      _currentPosition(0) {}

void ScrollingTextAnimation::setup(IDisplayDriver* display) {
    IAnimation::setup(display);
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
            if (textIndex >= 0 && textIndex < (int)_text.length()) {
                _display->setChar(i, _text[textIndex]);
            }
        }

        _display->writeDisplay();
        _currentPosition++;
    }
}
