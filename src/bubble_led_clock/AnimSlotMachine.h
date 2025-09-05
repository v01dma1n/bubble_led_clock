#ifndef ANIM_SLOT_MACHINE_H
#define ANIM_SLOT_MACHINE_H

#include "IAnim.h"
#include <string>

class SlotMachineAnimation : public IAnimation {
public:
    SlotMachineAnimation(std::string targetText, 
                         unsigned long lockDelay = 200, 
                         unsigned long holdTime = 3000, 
                         unsigned long spinDelay = 50);
    ~SlotMachineAnimation();
    void setup(IDisplayDriver* display);
    void update() override;
    bool isDone() override;

private:
    std::string _targetText;
    std::string _currentText;
    bool* _isLocked;
    int _lockedCount;
    
    unsigned long _lockDelay;
    unsigned long _lastLockTime;
    
    unsigned long _holdTime;
    unsigned long _lockingCompleteTime;
    
    unsigned long _spinDelay;
    unsigned long _lastSpinTime;
};

#endif // ANIM_SLOT_MACHINE_H

