#ifndef ANIM_SCROLLING_TEXT_H
#define ANIM_SCROLLING_TEXT_H

#include "IAnim.h"
#include <string>

class ScrollingTextAnimation : public IAnimation {
public:
    ScrollingTextAnimation(std::string text, unsigned long scrollDelay = 250);
    void setup(IDisplayDriver* display) override;
    void update() override;
    bool isDone() override;

private:
    std::string _text;
    unsigned long _scrollDelay;
    unsigned long _lastScrollTime;
    int _currentPosition;
};

#endif // ANIM_SCROLLING_TEXT_H

