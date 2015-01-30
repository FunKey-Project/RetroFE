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

#include <vector>

#include "../../SDL.h"
#include "../MenuNotifierInterface.h"
#include "../ViewInfo.h"
#include "../Animate/Tween.h"
#include "../Animate/TweenSet.h"
#include "../../Collection/Item.h"

class Component
{
public:
    Component();
    virtual ~Component();
    virtual void FreeGraphicsMemory();
    virtual void AllocateGraphicsMemory();
    virtual void LaunchEnter() {}
    virtual void LaunchExit() {}
    void TriggerEnterEvent();
    void TriggerExitEvent();
    void TriggerMenuEnterEvent();
    void TriggerMenuExitEvent();
    void TriggerMenuScrollEvent();
    void TriggerHighlightEvent(Item *selectedItem);
    bool IsIdle();
    bool IsHidden();
    bool IsWaiting();
    bool IsMenuScrolling();
    std::string GetCollectionName();
    void SetCollectionName(std::string collectionName);
    typedef std::vector<std::vector<Tween *> *> TweenSets;

    TweenSet *GetTweens() { return Tweens; }

    void SetTweens(TweenSet *set)
    {
        Tweens = set;
        CurrentAnimationState = IDLE; 
        CurrentTweenIndex = 0;
        CurrentTweenComplete = false;
        ElapsedTweenTime = 0;
    }

    virtual void Update(float dt);

    virtual void Draw();

    ViewInfo *GetBaseViewInfo()
    {
        return &BaseViewInfo;
    }
    void UpdateBaseViewInfo(ViewInfo &info)
    {
        BaseViewInfo = info;
    }

    bool IsScrollActive() const
    {
        return ScrollActive;
    }

    void SetScrollActive(bool scrollActive)
    {
        ScrollActive = scrollActive;
    }



protected:
    std::string CollectionName;
    Item *GetSelectedItem()
    {
        return SelectedItem;
    }
    enum AnimationState
    {
        IDLE,
        ENTER,
        HIGHLIGHT_EXIT,
        HIGHLIGHT_WAIT,
        HIGHLIGHT_ENTER,
        EXIT,
        MENU_ENTER,
        MENU_SCROLL,
        MENU_EXIT,
        HIDDEN
    };

    AnimationState CurrentAnimationState;
    bool EnterRequested;
    bool ExitRequested;
    bool MenuEnterRequested;
    bool MenuScrollRequested;
    bool MenuExitRequested;
    bool NewItemSelected;
    bool HighlightExitComplete;
    bool NewItemSelectedSinceEnter;
private:
    bool Animate(bool loop);
    bool IsTweenSequencingComplete();
    void ResetTweenSequence(std::vector<ViewInfo *> *tweens);
    TweenSet *Tweens;
    TweenSets *CurrentTweens;
    unsigned int CurrentTweenIndex;

    bool CurrentTweenComplete;
    ViewInfo BaseViewInfo;

    float ElapsedTweenTime;
    Tween *TweenInst;
    Item *SelectedItem;
    bool ScrollActive;
    SDL_Texture *BackgroundTexture;

};
