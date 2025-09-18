#include "display_manager.h"

DisplayManager::DisplayManager(IDisplayDriver& display) : _display(display) {}

void DisplayManager::begin() {
    _display.begin();
    _display.setBrightness(1);
}

void DisplayManager::setAnimation(std::unique_ptr<IAnimation> animation) {
    _currentAnimation = std::move(animation);
    if (_currentAnimation) {
        _currentAnimation->setup(&_display);
    }
}

void DisplayManager::update() {
    if (_currentAnimation) {
        _currentAnimation->update();
        if (_currentAnimation->isDone()) {
            _currentAnimation.reset(); // Clear the animation
            _display.clear();
            _display.writeDisplay();            
        }
    }
}

bool DisplayManager::isAnimationRunning() const {
    // The animation is running if the unique_ptr is not null
    return _currentAnimation != nullptr;
}

