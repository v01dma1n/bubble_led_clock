#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h> 
#include <memory> // For std::unique_ptr

#include "i_display_driver.h"
#include "i_animation.h"

class DisplayManager {
public:
    // The constructor takes a reference to our abstract display driver.
    DisplayManager(IDisplayDriver& display);

    void begin();
    void update(); // This should be called in the main loop()

    // Sets the animation to be played.
    // unique_ptr automatically handles memory management.
    void setAnimation(std::unique_ptr<IAnimation> animation);

    bool isAnimationRunning() const;

private:
    IDisplayDriver& _display; // Reference to the display driver
    std::unique_ptr<IAnimation> _currentAnimation; // The current animation
};

#endif // DISPLAY_MANAGER_H
