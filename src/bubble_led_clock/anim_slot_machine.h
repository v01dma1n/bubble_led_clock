#ifndef ANIM_SLOT_MACHINE_H
#define ANIM_SLOT_MACHINE_H

#include "i_animation.h"

#include <string>

class SlotMachineAnimation : public IAnimation {
public:
    SlotMachineAnimation(std::string targetText, 
                         unsigned long lockDelay = 200, 
                         unsigned long holdTime = 3000, 
                         unsigned long spinDelay = 50,
                         bool dotsWithPreviousChar = false);
    ~SlotMachineAnimation();
    void setup(IDisplayDriver* display);
    void update() override;
    bool isDone() override;

private:
  std::string _targetText;
    std::string _currentText;
    bool* _isLocked;
    unsigned long _lockDelay;
    unsigned long _holdTime;
    unsigned long _spinDelay;
    bool _dotsWithPreviousChar;

    unsigned long _lastLockTime;
    unsigned long _lastSpinTime;
    int _lockedCount;
    unsigned long _lockingCompleteTime;
};

#endif // ANIM_SLOT_MACHINE_H

