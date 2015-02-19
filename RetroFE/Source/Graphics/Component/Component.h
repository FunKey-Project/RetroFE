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
#include "../Animate/AnimationEvents.h"
#include "../../Collection/Item.h"

class Component
{
public:
    Component();
    Component(const Component &copy);
    virtual ~Component();
    virtual void FreeGraphicsMemory();
    virtual void AllocateGraphicsMemory();
    virtual void LaunchEnter() {}
    virtual void LaunchExit() {}
    void TriggerEnterEvent();
    void TriggerExitEvent();
    void TriggerMenuEnterEvent(int menuIndex = -1);
    void TriggerMenuExitEvent(int menuIndex = -1);
    void TriggerMenuScrollEvent();
    void TriggerHighlightEvent(Item *selectedItem);
    bool IsIdle();
    bool IsHidden();
    bool IsWaiting();
    bool IsMenuScrolling();
    std::string GetCollectionName();
    void SetCollectionName(std::string collectionName);

    virtual void Update(float dt);
    virtual void Draw();
    AnimationEvents *GetTweens();
    void SetTweens(AnimationEvents *set);
    void ForceIdle();
    ViewInfo *GetBaseViewInfo();
    void UpdateBaseViewInfo(ViewInfo &info);
    bool IsScrollActive() const;
    void SetScrollActive(bool scrollActive);

protected:
    Item *GetSelectedItem();
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
    int MenuEnterIndex;
    bool MenuScrollRequested;
    bool MenuExitRequested;
    int MenuExitIndex;
    bool NewItemSelected;
    bool HighlightExitComplete;
    bool NewItemSelectedSinceEnter;
private:

    bool Animate(bool loop);
    bool IsTweenSequencingComplete();

    AnimationEvents *Tweens;
    Animation *CurrentTweens;
    Item *SelectedItem;
    SDL_Texture *BackgroundTexture;

    unsigned int CurrentTweenIndex;
    bool CurrentTweenComplete;
    std::string CollectionName;
    ViewInfo BaseViewInfo;
    float ElapsedTweenTime;
    bool ScrollActive;
};
