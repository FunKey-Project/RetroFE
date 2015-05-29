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
#include "Component.h"
#include "../Animate/Tween.h"
#include "../ComponentItemBinding.h"
#include "../MenuNotifierInterface.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include <SDL2/SDL.h>


//todo: This scrolling implementation needs to be overhauled
//      It needs to have a common interface to support different menu types
//      (It was originally sandbox code that creeped into here)

class Configuration;
class Font;

class ScrollingList : public Component
{
public:
    enum ScrollDirection
    {
        ScrollDirectionBack,
        ScrollDirectionForward,
        ScrollDirectionIdle,

    };

    ScrollingList(Configuration &c,
                  float scaleX,
                  float scaleY,
                  Font *font,
                  std::string layoutKey,
                  std::string imageType);

    ScrollingList(const ScrollingList &copy);
    virtual ~ScrollingList();
    void TriggerMenuEnterEvent();
    void TriggerMenuExitEvent();

    bool AllocateTexture(ComponentItemBinding *s);
    void DeallocateTexture(ComponentItemBinding *s);
    void SetItems(std::vector<ComponentItemBinding *> *spriteList);
    void DestroyItems();
    void SetPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<AnimationEvents *> *tweenPoints);
    void SetScrollDirection(ScrollDirection direction);
    void SetScrollOrientation(bool horizontal);
    bool IsHorizontalScroll();
    void PageUp();
    void PageDown();
    void LetterUp();
    void LetterDown();
    bool IsIdle();
    void SetSelectedIndex(int selectedIndex);
    ComponentItemBinding *GetSelectedCollectionItemSprite();
    ComponentItemBinding *GetPendingCollectionItemSprite();
    ComponentItemBinding *GetPendingSelectedCollectionItemSprite();
    void AddComponentForNotifications(MenuNotifierInterface *c);
    void RemoveComponentForNotifications(MenuNotifierInterface *c);
    std::vector<ComponentItemBinding *> *GetCollectionItemSprites();
    void RemoveSelectedItem();
    void FreeGraphicsMemory();
    void Update(float dt);
    void Draw();
    void Draw(unsigned int layer);
    void SetScrollAcceleration(float value);
    void SetStartScrollTime(float value);

private:
    void Click(double nextScrollTime);
    void DeallocateSpritePoints();
    void AllocateSpritePoints();
    void UpdateSprite(unsigned int spriteIndex, unsigned int pointIndex, bool newScroll, float dt, double nextScrollTime);
    unsigned int GetNextTween(unsigned int currentIndex, std::vector<ViewInfo *> *list);
    void ResetTweens(Component *c, AnimationEvents *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime);

    enum ScrollState
    {
        ScrollStateActive,
        ScrollStatePageChange,
        ScrollStateStopping,
        ScrollStateIdle
    };

    std::vector<ComponentItemBinding *> *SpriteList;
    std::vector<ViewInfo *> *ScrollPoints;
    std::vector<AnimationEvents *> *TweenPoints;
    std::vector<MenuNotifierInterface *> NotificationComponents;
    float TweenEnterTime;
    bool Focus;

    unsigned int FirstSpriteIndex;
    unsigned int SelectedSpriteListIndex;
    bool ScrollStopRequested;
    bool NotifyAllRequested;
    ScrollDirection CurrentScrollDirection;
    ScrollDirection RequestedScrollDirection;
    ScrollState CurrentScrollState;
    float ScrollAcceleration;
    float StartScrollTime;
    float ScrollPeriod;

    int CircularIncrement(unsigned int index, unsigned int offset, std::vector<ComponentItemBinding *> *list);
    void CircularIncrement(unsigned &index, std::vector<ComponentItemBinding *> *list);
    void CircularDecrement(unsigned &index, std::vector<ComponentItemBinding *> *list);
    void CircularIncrement(unsigned &index, std::vector<ViewInfo *> *list);
    void CircularDecrement(unsigned &index, std::vector<ViewInfo *> *list);
    void UpdateOffset(float dt);

    std::string Collection;
    Configuration &Config;
    float ScaleX;
    float ScaleY;
    Font *FontInst;
    std::string LayoutKey;
    std::string ImageType;
    bool HorizontalScroll;
};

