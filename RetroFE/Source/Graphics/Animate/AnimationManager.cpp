#include "AnimationManager.h"
#include "Tween.h"

AnimationManager::~AnimationManager()
{
    AnimationChainsIt_T it = chains.begin();
    
    while(it != chains.end())
    {
        delete it->second;
        chains.erase(it);
        it = chains.begin();
    }
}

void AnimationManager::destroyChain(AnimationChain *chain) 
{
    AnimationChainsIt_T it = chains.find(chain);
    
    if(it != chains.end())
    {
        delete chain;
        chains.erase(it);
    }
}

void AnimationManager::update(float dt)
{
    for(AnimationChainsIt_T it = chains.begin(); it != chains.end(); )
    {
        AnimationChain *chain = it->first;
        AnimationChainsIt_T next = it;
        next++;
        
        if(updateChain(dt, chain)) 
        {
            chain->loopCount++;
            
            // reset the counter to continue iterating
            if(chain->loop) {
                chain->index = 0;
            }

            if(chain->autoDestroy) 
            {
                destroyChain(chain);
            }
        }
        
        it = next;
    }
}

bool AnimationManager::updateChain(float dt, AnimationChain *chain)
{
    if(chain->index >= chain->animations.size()) return true;
    
    Animation *a = chain->animations[chain->index];
    
    if(a->elapsedTime == 0) {
        a->start = a->component->info;
    }

    a->elapsedTime += dt;

    if(a->elapsedTime < a->duration) {

        ComponentData *endInfo = &a->end;
        ComponentData *startInfo = &a->start;
        ComponentData *currentInfo = &a->component->info;
        

        if(endInfo->isMaskSet(COMPONENT_DATA_X_MASK)) {
            currentInfo->x = (int)Tween::calculate(a->algorithm, startInfo->x, endInfo->x, a->duration, a->elapsedTime);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_Y_MASK)) {
            currentInfo->y = (int)Tween::calculate(a->algorithm, startInfo->y, endInfo->y, a->duration, a->elapsedTime);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_ALPHA_MASK)) {
            currentInfo->alpha = (float)Tween::calculate(a->algorithm, startInfo->alpha, endInfo->alpha, a->duration, a->elapsedTime);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_ROTATE_MASK)) {
            currentInfo->rotate = (float)Tween::calculate(a->algorithm, startInfo->rotate, endInfo->rotate, a->duration, a->elapsedTime);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_WIDTH_MASK)) {
            currentInfo->width = (int)Tween::calculate(a->algorithm, startInfo->width, endInfo->width, a->duration, a->elapsedTime);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_HEIGHT_MASK)) {
            currentInfo->height = (int)Tween::calculate(a->algorithm, startInfo->height, endInfo->height, a->duration, a->elapsedTime);
        }
    }
    else {
        a->elapsedTime = 0;
        chain->index++;
    }
    
    return (chain->index >= chain->animations.size());
}

AnimationChain *AnimationManager::start(Animation *a, bool loop, bool autoDestroy)
{
    return startChain(a, 1, loop, autoDestroy);
}

AnimationChain *AnimationManager::startChain(Animation *list, unsigned int size, bool loop, bool autoDestroy)
{
    AnimationChain *chain = new AnimationChain();
    
    for(unsigned int i = 0; i < size; ++i) {
        chain->animations.push_back(list);
        list++;
    }
    
    chain->index = 0;
    chain->loop = loop;
    chain->autoDestroy = autoDestroy;
    chains[chain] = chain;
    
    return chain;
}