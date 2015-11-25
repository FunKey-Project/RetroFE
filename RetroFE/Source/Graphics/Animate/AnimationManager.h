#pragma once

#include "Animation.h"
#include "AnimationChain.h"
#include <map>

class AnimationManager {
public:
    virtual ~AnimationManager();
    void destroyChain(AnimationChain *c);
    void update(float dt);
    bool updateChain(float dt, AnimationChain *chain);
    AnimationChain *start(Animation *a, bool loop, bool autoDestroy);
    AnimationChain *startChain(Animation *list, unsigned int size, bool loop, bool autoDestroy);
    
private:
    typedef std::map<AnimationChain *, AnimationChain *> AnimationChains_T;
    typedef AnimationChains_T::iterator AnimationChainsIt_T;
    AnimationChains_T chains;
};