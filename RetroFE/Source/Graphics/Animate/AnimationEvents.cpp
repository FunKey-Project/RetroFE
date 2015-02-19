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

#include "AnimationEvents.h"
#include <string>



AnimationEvents::AnimationEvents()
{
}

AnimationEvents::AnimationEvents(AnimationEvents &copy)
{
    for(std::map<std::string, std::map<int , Animation *> >::iterator it = copy.AnimationMap.begin(); it != copy.AnimationMap.end(); it++)
    {
        for(std::map<int, Animation *>::iterator it2 = (it->second).begin(); it2 != (it->second).end(); it2++)
        {
            Animation *t = new Animation(*it2->second);
            AnimationMap[it->first][it2->first] = t;
        }
    }
}

AnimationEvents::~AnimationEvents()
{
    Clear();
}

Animation *AnimationEvents::GetAnimation(std::string tween)
{
    return GetAnimation(tween, -1);
}

Animation *AnimationEvents::GetAnimation(std::string tween, int index)
{
    if(AnimationMap.find(tween) == AnimationMap.end())
    {
        AnimationMap[tween][-1] = new Animation();
    }

    if(AnimationMap[tween].find(index) == AnimationMap[tween].end())
    {
        index = -1;

        if(AnimationMap[tween].find(index) == AnimationMap[tween].end())
        {
            AnimationMap[tween][index] = new Animation();
        }
    }

    return AnimationMap[tween][index];
}

void AnimationEvents::SetAnimation(std::string tween, int index, Animation *animation)
{
    AnimationMap[tween][index] = animation;
}

void AnimationEvents::Clear()
{
    std::map<std::string, std::map<int, Animation *> >::iterator it = AnimationMap.begin();
    while(it != AnimationMap.end())
    {
        std::map<int, Animation *>::iterator it2 = (it->second).begin();
        while(it2 != (it->second).end())
        {
            delete it2->second;
            (it->second).erase(it2);
        }

        (it->second).clear();
        AnimationMap.erase(it);
        it = AnimationMap.begin();
    }

    AnimationMap.clear();
}




