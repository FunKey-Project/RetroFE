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
#include "../Page.h"
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
                  Page &p,
                  float scaleX,
                  float scaleY,
                  Font *font,
                  std::string layoutKey,
                  std::string imageType);

    ScrollingList(const ScrollingList &copy);
    virtual ~ScrollingList();
    void triggerEnterEvent();
    void triggerExitEvent();
    void triggerMenuEnterEvent(int menuIndex = -1);
    void triggerMenuExitEvent(int menuIndex = -1);
    void triggerHighlightEnterEvent(int menuIndex = -1);
    void triggerHighlightExitEvent(int menuIndex = -1);

    bool allocateTexture(unsigned int index, Item *i);
    void deallocateTexture(unsigned int index);
    void setItems(std::vector<Item *> *items);
    void destroyItems();
    void setPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<AnimationEvents *> *tweenPoints);
    void setScrollDirection(ScrollDirection direction);
    unsigned int getSelectedIndex();
    unsigned int getSize(); 
    void pageUp();
    void pageDown();
    void letterUp();
    void letterDown();
    void letterChange(bool increment);
    void random();
    bool isIdle();
    unsigned int getScrollOffsetIndex();
    void setScrollOffsetIndex(unsigned int index);
    void setSelectedIndex(int selectedIndex);
    Item *getItemByOffset(int offset);
    Item *getSelectedItem();
    void freeGraphicsMemory();
    void update(float dt);
    void draw();
    void draw(unsigned int layer);
    void setScrollAcceleration(float value);
    void setStartScrollTime(float value);
    bool horizontalScroll;
    void deallocateSpritePoints();
    void allocateSpritePoints();


private:
    void click(double nextScrollTime);
    void resetTweens(Component *c, AnimationEvents *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime);
    unsigned int loopIncrement(unsigned int offset, unsigned int i, unsigned int size);
    unsigned int loopDecrement(unsigned int offset, unsigned int i, unsigned int size);

    enum ScrollState
    {
        ScrollStateActive,
        ScrollStatePageChange,
        ScrollStateStopping,
        ScrollStateIdle
    };

    std::vector<Component *> *spriteList_;
    std::vector<ViewInfo *> *scrollPoints_;
    std::vector<AnimationEvents *> *tweenPoints_;

    unsigned int itemIndex_;
    unsigned int componentIndex_;
    unsigned int selectedOffsetIndex_;

    ScrollDirection currentScrollDirection_;
    ScrollDirection requestedScrollDirection_;
    ScrollState currentScrollState_;

    float scrollAcceleration_;
    float startScrollTime_;
    float scrollPeriod_;
    
    Configuration &config_;
    float scaleX_;
    float scaleY_;
    Font *fontInst_;
    std::string layoutKey_;
    std::string imageType_;

        
    std::vector<Item *> *items_;
    std::vector<Component *> components_;
    
    
};

