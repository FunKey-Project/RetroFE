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

#include "Animation.h"
#include <string>

Animation::Animation()
{
}

Animation::Animation(Animation &copy)
{
    for(std::vector<TweenSet *>::iterator it = copy.AnimationVector.begin(); it != copy.AnimationVector.end(); it++)
    {
        Push(new TweenSet(**it));
    }
}
Animation::~Animation()
{
    Clear();
}

void Animation::Push(TweenSet *set)
{
    AnimationVector.push_back(set);
}

void Animation::Clear()
{
    std::vector<TweenSet *>::iterator it = AnimationVector.begin();
    while(it != AnimationVector.end())
    {
        delete *it;
        AnimationVector.erase(it);
        it = AnimationVector.begin();
    }

    AnimationVector.clear();
}

std::vector<TweenSet *> *Animation::GetTweenSets()
{
    return &AnimationVector;
}

TweenSet *Animation::GetTweenSet(unsigned int index)
{
    return AnimationVector[index];
}


unsigned int Animation::GetSize()
{
    return AnimationVector.size();
}
