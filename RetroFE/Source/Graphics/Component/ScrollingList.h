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
    void triggerMenuEnterEvent();
    void triggerMenuExitEvent();

    bool allocateTexture(ComponentItemBinding *s);
    void deallocateTexture(ComponentItemBinding *s);
    void setItems(std::vector<ComponentItemBinding *> *spriteList);
    void destroyItems();
    void setPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<AnimationEvents *> *tweenPoints);
    void setScrollDirection(ScrollDirection direction);
    void pageUp();
    void pageDown();
    void letterUp();
    void letterDown();
    bool isIdle();
    unsigned int getScrollOffsetIndex();
    void setScrollOffsetIndex(unsigned int index);
    void setSelectedIndex(int selectedIndex);
    ComponentItemBinding *getSelectedCollectionItemSprite();
    ComponentItemBinding *getPendingCollectionItemSprite();
    ComponentItemBinding *getPendingSelectedCollectionItemSprite();
    void addComponentForNotifications(MenuNotifierInterface *c);
    void removeComponentForNotifications(MenuNotifierInterface *c);
    std::vector<ComponentItemBinding *> *getCollectionItemSprites();
    void removeSelectedItem();
    void freeGraphicsMemory();
    void update(float dt);
    void draw();
    void draw(unsigned int layer);
    void setScrollAcceleration(float value);
    void setStartScrollTime(float value);
    bool horizontalScroll;

private:
    void click(double nextScrollTime);
    void deallocateSpritePoints();
    void allocateSpritePoints();
    void updateSprite(unsigned int spriteIndex, unsigned int pointIndex, bool newScroll, float dt, double nextScrollTime);
    unsigned int getNextTween(unsigned int currentIndex, std::vector<ViewInfo *> *list);
    void resetTweens(Component *c, AnimationEvents *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime);

    enum ScrollState
    {
        ScrollStateActive,
        ScrollStatePageChange,
        ScrollStateStopping,
        ScrollStateIdle
    };

    std::vector<ComponentItemBinding *> *spriteList_;
    std::vector<ViewInfo *> *scrollPoints_;
    std::vector<AnimationEvents *> *tweenPoints_;
    std::vector<MenuNotifierInterface *> notificationComponents_;
    float tweenEnterTime_;
    bool focus_;

    unsigned int firstSpriteIndex_;
    unsigned int selectedSpriteListIndex_;
    bool scrollStopRequested_;
    bool notifyAllRequested_;
    ScrollDirection currentScrollDirection_;
    ScrollDirection requestedScrollDirection_;
    ScrollState currentScrollState_;
    float scrollAcceleration_;
    float startScrollTime_;
    float scrollPeriod_;

    int circularIncrement(unsigned int index, unsigned int offset, std::vector<ComponentItemBinding *> *list);
    void circularIncrement(unsigned &index, std::vector<ComponentItemBinding *> *list);
    void circularDecrement(unsigned &index, std::vector<ComponentItemBinding *> *list);
    void circularIncrement(unsigned &index, std::vector<ViewInfo *> *list);
    void circularDecrement(unsigned &index, std::vector<ViewInfo *> *list);
    void updateOffset(float dt);

    std::string collection_;
    Configuration &config_;
    float scaleX_;
    float scaleY_;
    Font *fontInst_;
    std::string layoutKey_;
    std::string imageType_;
};

