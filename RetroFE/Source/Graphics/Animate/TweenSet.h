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
#pragma once

#include "Tween.h"
#include <string>
#include <vector>
#include <map>



class TweenSet
{
public:
    ~TweenSet();
    typedef std::vector<std::vector<Tween *> *> TweenSets;
    //todo: delete the tweens in a destructor

    TweenSets *GetOnEnterTweens();
    TweenSets *GetOnExitTweens();
    TweenSets *GetOnIdleTweens();
    TweenSets *GetOnHighlightEnterTweens();
    TweenSets *GetOnHighlightExitTweens();
    TweenSets *GetOnMenuScrollTweens();
    TweenSets *GetOnMenuEnterTweens();
    TweenSets *GetOnMenuExitTweens();
    TweenSets *GetOnMenuEnterTweens(int index);
    TweenSets *GetOnMenuExitTweens(int index);
    void SetOnMenuEnterTweens(int index, TweenSets *set);
    void SetOnMenuExitTweens(int index, TweenSets *set);

private:
    TweenSets OnEnterTweens;
    TweenSets OnExitTweens;
    TweenSets OnIdleTweens;
    TweenSets OnHighlightEnterTweens;
    TweenSets OnHighlightExitTweens;
    TweenSets OnMenuScrollTweens;
    std::map<int, TweenSets *> OnMenuEnterTweens;
    std::map<int, TweenSets *> OnMenuExitTweens;

};
