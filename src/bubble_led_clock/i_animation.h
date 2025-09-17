#ifndef I_ANIM_H
#define I_ANIM_H

#include "i_display_driver.h"

class IAnimation {
public:
    virtual ~IAnimation() {}

    // Setup is called once when the animation starts
    virtual void setup(IDisplayDriver* display) {
        _display = display;
    }

    // Update is called on every loop iteration to draw the next frame
    virtual void update() = 0;

    // Returns true if the animation has finished
    virtual bool isDone() = 0;

protected:
    IDisplayDriver* _display;
};

#endif // I_ANIM_H