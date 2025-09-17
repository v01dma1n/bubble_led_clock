#ifndef ANIM_MATRIX_H
#define ANIM_MATRIX_H

#include <string>
#include <vector>

#include "i_animation.h"

class MatrixAnimation : public IAnimation {
public:
    MatrixAnimation(std::string targetText,
                    unsigned long revealDelay = 200,
                    unsigned long holdTime = 2000,
                    unsigned long rainDelay = 50,
                    bool dotsWithPreviousChar = false);
    
    ~MatrixAnimation() override = default;

    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _targetText;
    bool _dotsWithPreviousChar;
    
    // Parsed text and dot states
    std::string _parsedTargetText;
    std::vector<bool> _dotState;

    // Timing controls
    unsigned long _revealDelay;
    unsigned long _holdTime;
    unsigned long _rainDelay;

    // State variables
    unsigned long _lastRainTime;
    unsigned long _lastRevealTime;
    unsigned long _revealCompleteTime;
    int _revealedCount;
    
    std::vector<std::vector<float>> _rainPos;
};

#endif // ANIM_MATRIX_H