#ifndef ANIM_MATRIX_H
#define ANIM_MATRIX_H

#include "IAnim.h"
#include <string>
#include <vector>

class MatrixAnimation : public IAnimation {
public:
    /**
     * @brief Constructor for the Matrix-style animation.
     * @param targetText The final text to be revealed.
     * @param revealDuration The time in ms for the raining/revealing phase.
     * @param holdTime The time in ms to display the final text after reveal.
     * @param rainSpeed The speed of the falling segments (higher is faster).
     */
    MatrixAnimation(std::string targetText, 
                    unsigned long revealDuration = 2000, 
                    unsigned long holdTime = 2000, 
                    float rainSpeed = 0.005f);
    
    ~MatrixAnimation() override = default;

    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    // Helper to get segment data for a character
    uint8_t getSegmentsForChar(char c);

    std::string _targetText;
    unsigned long _revealDuration;
    unsigned long _holdTime;
    float _rainSpeed;

    unsigned long _startTime;
    bool _holdPhaseStarted;

    // Vectors to store segment data for each display position
    std::vector<uint8_t> _targetSegments;
    std::vector<uint8_t> _revealedSegments;
    
    // Tracks the vertical position of the three "rain columns" for each digit
    std::vector<std::vector<float>> _rainPos;
};

#endif // ANIM_MATRIX_H