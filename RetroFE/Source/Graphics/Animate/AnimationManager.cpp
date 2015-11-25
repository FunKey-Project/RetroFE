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

        if(endInfo->isMaskSet(COMPONENT_DATA_X_MASK)) {
            endInfo->x = (int)Tween::calculate(a->algorithm, a->elapsedTime, a->duration, startInfo->x, endInfo->x - startInfo->x);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_Y_MASK)) {
            endInfo->y = (int)Tween::calculate(a->algorithm, a->elapsedTime, a->duration, startInfo->y, endInfo->y - startInfo->y);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_ALPHA_MASK)) {
            endInfo->alpha = (float)Tween::calculate(a->algorithm, a->elapsedTime, a->duration, startInfo->alpha, endInfo->alpha - startInfo->alpha);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_ROTATE_MASK)) {
            endInfo->rotate = (float)Tween::calculate(a->algorithm, a->elapsedTime, a->duration, startInfo->rotate, endInfo->rotate - startInfo->rotate);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_WIDTH_MASK)) {
            endInfo->width = (int)Tween::calculate(a->algorithm, a->elapsedTime, a->duration, startInfo->width, endInfo->width - startInfo->width);
        }
        if(endInfo->isMaskSet(COMPONENT_DATA_HEIGHT_MASK)) {
            endInfo->height = (int)Tween::calculate(a->algorithm, a->elapsedTime, a->duration, startInfo->height, endInfo->height - startInfo->height);
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