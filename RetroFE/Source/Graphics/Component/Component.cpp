/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "Component.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"

Component::Component()
    : startInfo(&info)
    , endInfo(&info)
    , elapsedTime(0)
    , loop(false)
    , start(false)
{
}
void Component::addAnimation(const ComponentData &newInfo)
{
    start = false;
    animations.push_back(newInfo);
    elapsedTime = 0;
}

void Component::animate(bool loop) 
{
    this->loop = loop;
    start = true;
    elapsedTime = 0;
    animationIndex = -1;

    startInfo = &info;
    endInfo = &info;

    if(animations.size() > 0)
    {
        endInfo = &animations[0];
    }
}

double linear(double t, double d, double b, double c)
{
    if(d == 0) return b;
    return c*t/d + b;
}
void Component::update(float dt)
{
    elapsedTime += dt;  
    bool done = false;
    while(elapsedTime && endInfo->duration && elapsedTime >= endInfo->duration) {
        elapsedTime -= endInfo->duration;

        // don't animate if no tweens exist
        if(animations.size() == 0) {
            startInfo = &info;
            endInfo = &info;
        }
        else if(loop) {
            animationIndex = (animationIndex + 1) % animations.size();
            unsigned int nextAnimationIndex = (animationIndex + 1) % animations.size();
            done = (animationIndex + 1 >= animations.size());
            startInfo = &animations[animationIndex];
            endInfo = &animations[nextAnimationIndex];
        }
    }

    if(start) {
        info.x = (int)linear(elapsedTime, endInfo->duration, startInfo->x, endInfo->x - startInfo->x);
        info.y = (int)linear(elapsedTime, endInfo->duration, startInfo->y, endInfo->y - startInfo->y);
        info.alpha = (float)linear(elapsedTime, endInfo->duration, startInfo->alpha, endInfo->alpha - startInfo->alpha);
        info.rotate = (float)linear(elapsedTime, endInfo->duration, startInfo->rotate, endInfo->rotate - startInfo->rotate);
        info.width = (int)linear(elapsedTime, endInfo->duration, startInfo->width, endInfo->width - startInfo->width);
        info.height = (int)linear(elapsedTime, endInfo->height, startInfo->height, endInfo->height - startInfo->height);

        if(!loop && done) {
            start = false;
        }
    }
}

