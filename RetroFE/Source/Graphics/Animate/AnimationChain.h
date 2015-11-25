#pragma once
#include "Animation.h"
#include <vector>

class AnimationChain {
public:
    AnimationChain();
    virtual ~AnimationChain();
    typedef std::vector<Animation *> AnimationList_T;
    typedef AnimationList_T::iterator AnimationListIt_T;
    
    AnimationList_T animations;
    unsigned int index;
    bool loop;
    unsigned int loopCount;
    bool autoDestroy;
};
