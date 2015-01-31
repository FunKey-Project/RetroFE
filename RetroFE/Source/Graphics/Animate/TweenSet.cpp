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
    std::map<int, TweenSets *>::iterator it;
    
    it = OnMenuEnterTweens.begin();
    while(it != OnMenuEnterTweens.end())
    {
        delete it->second;
        OnMenuEnterTweens.erase(it);
        it = OnMenuEnterTweens.begin();
    }

    it = OnMenuExitTweens.begin();
    while(it != OnMenuExitTweens.end())
    {
        delete it->second;
        OnMenuExitTweens.erase(it);
        it = OnMenuExitTweens.begin();
    }
}

TweenSet::TweenSets *TweenSet::GetOnEnterTweens()
{
    return &OnEnterTweens;
}
TweenSet::TweenSets *TweenSet::GetOnExitTweens()
{
    return &OnExitTweens;
}
TweenSet::TweenSets *TweenSet::GetOnIdleTweens()
{
    return &OnIdleTweens;
}
TweenSet::TweenSets *TweenSet::GetOnHighlightEnterTweens()
{
    return &OnHighlightEnterTweens;
}
TweenSet::TweenSets *TweenSet::GetOnHighlightExitTweens()
{
    return &OnHighlightExitTweens;
}

TweenSet::TweenSets *TweenSet::GetOnMenuEnterTweens()
{
    return GetOnMenuEnterTweens(-1);
}


TweenSet::TweenSets *TweenSet::GetOnMenuEnterTweens(int index)
{
    if(OnMenuEnterTweens.find(index) == OnMenuEnterTweens.end())
    {
        index = -1;

        if(OnMenuEnterTweens.find(index) == OnMenuEnterTweens.end())
        {
            TweenSets *set = new TweenSets();
            OnMenuEnterTweens[index] = set;
        }
    }

    return OnMenuEnterTweens[index];
}

TweenSet::TweenSets *TweenSet::GetOnMenuScrollTweens()
{
    return &OnMenuScrollTweens;
}

TweenSet::TweenSets *TweenSet::GetOnMenuExitTweens()
{
    return GetOnMenuExitTweens(-1);
}


TweenSet::TweenSets *TweenSet::GetOnMenuExitTweens(int index)
{
    if(OnMenuExitTweens.find(index) == OnMenuExitTweens.end())
    {
        index = -1;

        if(OnMenuExitTweens.find(index) == OnMenuExitTweens.end())
        {
            TweenSets *set = new TweenSets();
            OnMenuExitTweens[index] = set;
        }
    }

    return OnMenuExitTweens[index];
}

void TweenSet::SetOnMenuEnterTweens(int index, TweenSets *set)
{
    OnMenuEnterTweens[index] = set;
}

void TweenSet::SetOnMenuExitTweens(int index, TweenSets *set)
{
    OnMenuExitTweens[index] = set;
}


