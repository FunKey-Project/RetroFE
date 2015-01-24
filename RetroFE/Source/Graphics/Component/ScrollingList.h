/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
        SDL_Color fontColor, 
        std::string layoutKey, 
        std::string imageType);
    virtual ~ScrollingList();
    void TriggerMenuEnterEvent();
    void TriggerMenuExitEvent();

    void AllocateTexture(ComponentItemBinding *s);
    void DeallocateTexture(ComponentItemBinding *s);
    void SetItems(std::vector<ComponentItemBinding *> *spriteList);
    void SetPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<TweenSet *> *tweenPoints);
    void SetScrollDirection(ScrollDirection direction);
    void PageUp();
    void PageDown();
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

private:
    void Click();
    unsigned int GetNextTween(unsigned int currentIndex, std::vector<ViewInfo *> *list);
    bool IsScrollChangedStarted;
    bool IsScrollChangedSignalled;
    bool IsScrollChangedComplete;

    enum ScrollState
    {
        ScrollStateActive,
        ScrollStatePageChange,
        ScrollStateStopping,
        ScrollStateIdle
    };

    std::vector<ComponentItemBinding *> *SpriteList;
    std::vector<ViewInfo *> *ScrollPoints;
     std::vector<TweenSet *> *TweenPoints;
    std::vector<MenuNotifierInterface *> NotificationComponents;
    float TweenEnterTime;
    bool Focus;

    unsigned int FirstSpriteIndex;
    unsigned int SelectedSpriteListIndex;
    float CurrentAnimateTime;
    float ScrollTime;

    ScrollDirection CurrentScrollDirection;
    ScrollDirection RequestedScrollDirection;
    ScrollState CurrentScrollState;
    float ScrollAcceleration;
    float ScrollVelocity;

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
    SDL_Color FontColor;
    std::string LayoutKey;
    std::string ImageType;
    unsigned int MaxLayer;
};

