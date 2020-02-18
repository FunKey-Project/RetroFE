#pragma once

#include "Component.h"
#include <SDL/SDL.h>
#include <string>

class Battery : public Component
{
public:
	Battery(Page &p, float scaleX, float scaleY, float reloadPeriod, SDL_Color fontColor);
    virtual ~Battery();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void drawBattery();
    void update(float dt);
    void draw();
    bool mustRender();

protected:
    SDL_Surface *texture_;
    SDL_Surface *texture_prescaled_;
    uint32_t 	fontColor_;
    float 		scaleX_;
    float 		scaleY_;
    float		reloadPeriod_;

    static float	currentWaitTime_;
    static bool 	mustRender_;
    static int 		percentage_;
    static int 		prevPercentage_;
    static bool 	charging_;
};
