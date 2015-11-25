#include "AnimationChain.h"

AnimationChain::AnimationChain()
: index(0)
, loop(false)
, loopCount(0)
, autoDestroy(false)
{
}

AnimationChain::~AnimationChain() {
    for (AnimationList_T::iterator it = animations.begin(); it != animations.end(); it++) {
        delete *it;
    }
    
    animations.clear();
}