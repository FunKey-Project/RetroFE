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

TweenSet::TweenSet()
{
}

TweenSet::TweenSet(TweenSet &copy)
{
    for(std::vector<Tween *>::iterator it = copy.Set.begin(); it != copy.Set.end(); it++)
    {
        Tween *t = new Tween(**it);
        Set.push_back(t);
    }
}

TweenSet::~TweenSet()
{
    Clear();
}

void TweenSet::Push(Tween *tween)
{
    Set.push_back(tween);
}

void TweenSet::Clear()
{
    std::vector<Tween *>::iterator it = Set.begin();
    while(it != Set.end())
    {
        delete *it;
        Set.erase(it);
        it = Set.begin();
    }
}
std::vector<Tween *> *TweenSet::GetTweens()
{
    return &Set;
}

Tween *TweenSet::GetTween(unsigned int index)
{
    return Set[index];
}


unsigned int TweenSet::GetSize()
{
    return Set.size();
}
