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

#include "TweenSet.h"

TweenSet::~TweenSet() 
{
    DestroyTweens();
}

void TweenSet::DestroyTweens()
{
    std::map<std::string, std::map<int, TweenSets *>>::iterator it = TweenMap.begin();

    while(it != TweenMap.end())
    {
        std::map<int, TweenSets *>::iterator it2 = (it->second).begin();

        while(it2 != (it->second).end())
        {
            delete it2->second;
            (it->second).erase(it2);
            it2 = (it->second).begin();
        }

        it =TweenMap.begin();
    }
}

TweenSet::TweenSets *TweenSet::GetTween(std::string tween)
{
    return GetTween(tween, -1);
}

TweenSet::TweenSets *TweenSet::GetTween(std::string tween, int index)
{
    return FindTween(TweenMap[tween], index);
}

void TweenSet::SetTween(std::string tween, int index, TweenSets *set)
{
    TweenMap[tween][index] = set;
}

TweenSet::TweenSets *TweenSet::FindTween(std::map<int, TweenSets *> &tweens, int index)
{
    if(tweens.find(index) == tweens.end())
    {
        index = -1;

        if(tweens.find(index) == tweens.end())
        {
            TweenSets *set = new TweenSets();
            tweens[index] = set;
        }
    }

    return tweens[index];
}


