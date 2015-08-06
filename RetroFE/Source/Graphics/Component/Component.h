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
    virtual void freeGraphicsMemory();
    virtual void allocateGraphicsMemory();
    virtual void launchEnter() {}
    virtual void launchExit() {}
    void triggerEnterEvent();
    void triggerExitEvent();
    void triggerMenuEnterEvent(int menuIndex = -1);
    void triggerMenuExitEvent(int menuIndex = -1);
    void triggerMenuScrollEvent();
    void triggerHighlightEvent(Item *selectedItem);
    void triggerPlaylistChangeEvent(std::string name);
    bool isIdle();
    bool isHidden();
    bool isWaiting();
    bool isMenuScrolling();

    virtual void update(float dt);
    virtual void draw();
    void setTweens(AnimationEvents *set);
    void forceIdle();
    ViewInfo baseViewInfo;
    std::string collectionName;
    bool scrollActive;

protected:
    Item *getSelectedItem();
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

    AnimationState currentAnimationState;
    bool enterRequested;
    bool exitRequested;
    bool menuEnterRequested;
    int menuEnterIndex;
    bool menuScrollRequested;
    bool menuExitRequested;
    int menuExitIndex;
    bool newItemSelected;
    bool playlistChanged;
    std::string playlistName;
    bool highlightExitComplete;
    bool newItemSelectedSinceEnter;
private:

    bool animate(bool loop);
    bool tweenSequencingComplete();

    AnimationEvents *tweens_;
    Animation *currentTweens_;
    Item *selectedItem_;
    SDL_Texture *backgroundTexture_;

    unsigned int currentTweenIndex_;
    bool currentTweenComplete_;
    float elapsedTweenTime_;
};
