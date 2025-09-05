#ifndef CLOCK_H
#define CLOCK_H


#include <Arduino.h> 
#include "IDispDriver.h"
#include "IAnim.h"
#include <memory> // For std::unique_ptr

class Clock {
public:
    // The constructor takes a reference to our abstract display driver.
    Clock(IDisplayDriver& display);

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

#endif // CLOCK_H
