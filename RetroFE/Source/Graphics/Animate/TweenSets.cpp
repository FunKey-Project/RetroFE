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

#include "TweenSets.h"

TweenSets::~TweenSets() 
{
    DestroyTweens();
}

void TweenSets::DestroyTweens()
{
    std::map<std::string, std::map<int, TweenAttributes *> >::iterator it = TweenMap.begin();

    while(it != TweenMap.end())
    {
        std::map<int, TweenAttributes *>::iterator it2 = (it->second).begin();

        while(it2 != (it->second).end())
        {
            delete it2->second;
            (it->second).erase(it2);
            it2 = (it->second).begin();
        }

        it =TweenMap.begin();
    }
}

TweenSets::TweenAttributes *TweenSets::GetTween(std::string tween)
{
    return GetTween(tween, -1);
}

TweenSets::TweenAttributes *TweenSets::GetTween(std::string tween, int index)
{
    return FindTween(TweenMap[tween], index);
}

void TweenSets::SetTween(std::string tween, int index, TweenAttributes *set)
{
    TweenMap[tween][index] = set;
}

TweenSets::TweenAttributes *TweenSets::FindTween(std::map<int, TweenAttributes *> &tweens, int index)
{
    if(tweens.find(index) == tweens.end())
    {
        index = -1;

        if(tweens.find(index) == tweens.end())
        {
            TweenAttributes *set = new TweenAttributes();
            tweens[index] = set;
        }
    }

    return tweens[index];
}


