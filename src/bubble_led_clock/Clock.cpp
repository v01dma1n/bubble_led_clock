#define ESP32DEBUGGING
#include <ESP32Logger.h>

#include "Clock.h"


Clock::Clock(IDisplayDriver& display) : _display(display) {}

void Clock::begin() {
    _display.begin();
    _display.setBrightness(5);
}

void Clock::setAnimation(std::unique_ptr<IAnimation> animation) {
    _currentAnimation = std::move(animation);
    if (_currentAnimation) {
        _currentAnimation->setup(&_display);
    }
}

void Clock::update() {
    if (_currentAnimation) {
        _currentAnimation->update();
        if (_currentAnimation->isDone()) {
            _currentAnimation.reset(); // Clear the animation
        }
    }
}

bool Clock::isAnimationRunning() const {
    // The animation is running if the unique_ptr is not null
    return _currentAnimation != nullptr;
}