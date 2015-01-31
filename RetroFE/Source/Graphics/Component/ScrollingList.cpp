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

#include "../Animate/Tween.h"
#include "../Animate/TweenTypes.h"
#include "../ComponentItemBinding.h"
#include "../Font.h"
#include "ScrollingList.h"
#include "ImageBuilder.h"
#include "VideoBuilder.h"
#include "VideoComponent.h"
#include "ReloadableMedia.h"
#include "Text.h"
#include "../../Database/Configuration.h" // todo: decouple the GUI from the data
#include "../../Collection/Item.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../ViewInfo.h"
#include <math.h>
#include <SDL2/SDL_image.h>
#include <sstream>


//todo: remove coupling from configuration data (if possible)
ScrollingList::ScrollingList(Configuration &c,
                             float scaleX,
                             float scaleY,
                             Font *font,
                             SDL_Color fontColor,
                             std::string layoutKey,
                             std::string imageType)
    : SpriteList(NULL)
    , ScrollPoints(NULL)
    , TweenEnterTime(0)
    , FirstSpriteIndex(0)
    , SelectedSpriteListIndex(0)
    , ScrollStopRequested(true)
    , CurrentScrollDirection(ScrollDirectionIdle)
    , RequestedScrollDirection(ScrollDirectionIdle)
    , CurrentScrollState(ScrollStateIdle)
    , ScrollAcceleration(6)  // todo: make configurable
    , ScrollPeriod(0)
    , Config(c)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
    , FontInst(font)
    , FontColor(fontColor)
    , LayoutKey(layoutKey)
    , ImageType(imageType)
    , MaxLayer(0)
    , Focus(false)
{
}

ScrollingList::~ScrollingList()
{
    DestroyItems();
}

void ScrollingList::SetItems(std::vector<ComponentItemBinding *> *spriteList)
{
    SpriteList = spriteList;
    FirstSpriteIndex = 0;

    if(!SpriteList) { return; }
    unsigned int originalSize = SpriteList->size();

    // loop the scroll points if there are not enough, the +2 represents the head and tail nodes (for when the item is allocated)
    while(ScrollPoints && ScrollPoints->size()+2 > SpriteList->size() && SpriteList->size() > 0)
    {
        for(unsigned int i = 0; i < originalSize; ++i)
        {
            Item *newItem = new Item();
            Item *originalItem = SpriteList->at(i)->GetCollectionItem();

            *newItem = *originalItem;
            ComponentItemBinding *newSprite = new ComponentItemBinding(newItem);
            SpriteList->push_back(newSprite);
        }
    }

    for(unsigned int i = 0; ScrollPoints && i < SelectedSpriteListIndex; ++i)
    {
        CircularDecrement(FirstSpriteIndex, SpriteList);
    }

    AllocateSpritePoints();
}

void ScrollingList::DeallocateSpritePoints()
{
    if(!SpriteList) { return; }

    unsigned int spriteIndex = FirstSpriteIndex;

    for(unsigned int i = 0; i < ScrollPoints->size(); ++i)
    {
        DeallocateTexture(SpriteList->at(spriteIndex));
        CircularIncrement(spriteIndex, SpriteList);
    }
}

void ScrollingList::AllocateSpritePoints()
{
    if(!ScrollPoints) { return; }
    if(!SpriteList) { return; }
    if(SpriteList->size() == 0) { return; }
    if(!TweenPoints) { return; }

    unsigned int spriteIndex = FirstSpriteIndex;

    for(unsigned int i = 0; i < ScrollPoints->size(); ++i)
    {
        AllocateTexture(SpriteList->at(spriteIndex));
        Component *c = SpriteList->at(spriteIndex)->GetComponent();
        ViewInfo *currentViewInfo = ScrollPoints->at(i);
        unsigned int nextI = GetNextTween(i, ScrollPoints);
        ViewInfo *nextViewInfo = ScrollPoints->at(nextI);

        ResetTweens(c, TweenPoints->at(i), currentViewInfo, nextViewInfo, 0);

        CircularIncrement(spriteIndex, SpriteList);
    }
}

void ScrollingList::DestroyItems()
{
    if(!SpriteList) { return; }
    std::vector<ComponentItemBinding *>::iterator it  = SpriteList->begin();

    while(it != SpriteList->end())
    {
        if(*it != NULL)
        {
            DeallocateTexture(*it);

            if((*it)->GetCollectionItem())
            {
                delete (*it)->GetCollectionItem();
            }

            delete *it;
        }


        SpriteList->erase(it);

        it = SpriteList->begin();
    }

    delete SpriteList;
    SpriteList = NULL;
}


void ScrollingList::SetPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<TweenSet *> *tweenPoints)
{
    ScrollPoints = scrollPoints;
    TweenPoints = tweenPoints;

    for(unsigned int i = 0; i != scrollPoints->size(); ++i)
    {
        ViewInfo *info = scrollPoints->at(i);
        MaxLayer = (MaxLayer < info->GetLayer()) ? MaxLayer : info->GetLayer();
    }

}

void ScrollingList::SetSelectedIndex(int selectedIndex)
{
    SelectedSpriteListIndex = selectedIndex;

    for(unsigned int i = 0; SpriteList && ScrollPoints && i < SelectedSpriteListIndex; ++i)
    {
        CircularDecrement(FirstSpriteIndex, SpriteList);
    }
}

void ScrollingList::Click(double nextScrollTime)
{
    if(!SpriteList) { return; }
    if(SpriteList->size() == 0) { return; }

    unsigned int listSize = ScrollPoints->size();
    unsigned int end = CircularIncrement(FirstSpriteIndex, listSize - 1, SpriteList);
    unsigned int allocSpriteIndex = 0;
    unsigned int deallocSpriteIndex = 0;
    unsigned int allocPoint = 0;

    if(CurrentScrollDirection == ScrollDirectionBack)
    {
        deallocSpriteIndex = end;
        CircularDecrement(FirstSpriteIndex, SpriteList);
        allocSpriteIndex = FirstSpriteIndex;
        allocPoint = 0;
    }
    else if(CurrentScrollDirection == ScrollDirectionForward)
    {
        deallocSpriteIndex = FirstSpriteIndex;
        CircularIncrement(FirstSpriteIndex, SpriteList);
        allocSpriteIndex = CircularIncrement(FirstSpriteIndex, listSize - 1, SpriteList);
        allocPoint = listSize - 1;
    }
    else
    {
        return;
    }

    DeallocateTexture(SpriteList->at(deallocSpriteIndex));
    AllocateTexture(SpriteList->at(allocSpriteIndex));

    Component *c = SpriteList->at(allocSpriteIndex)->GetComponent();
    ViewInfo *currentViewInfo = ScrollPoints->at(allocPoint);
    unsigned int nextI = GetNextTween(allocPoint, ScrollPoints);
    ViewInfo *nextViewInfo = ScrollPoints->at(nextI);

    ResetTweens(c, TweenPoints->at(allocPoint), currentViewInfo, nextViewInfo, nextScrollTime);
}

unsigned int ScrollingList::GetNextTween(unsigned int currentIndex, std::vector<ViewInfo *> *list)
{
    if(CurrentScrollDirection == ScrollDirectionForward)
    {
        CircularDecrement(currentIndex, list);
    }
    else if(CurrentScrollDirection == ScrollDirectionBack)
    {
        CircularIncrement(currentIndex, list);
    }

    return currentIndex;
}

void ScrollingList::PageUp()
{
    DeallocateSpritePoints();

    if(SpriteList && ScrollPoints && ScrollPoints->size() > 2)
    {
        ScrollPeriod = 0;
        unsigned int counts = ScrollPoints->size() - 2;

        for(unsigned int i = 0; i < counts; i++)
        {
            CircularDecrement(FirstSpriteIndex, SpriteList);
        }
    }

    AllocateSpritePoints();

//    CurrentScrollState = ScrollStatePageChange;
}

void ScrollingList::PageDown()
{
    DeallocateSpritePoints();

    if(SpriteList && ScrollPoints && ScrollPoints->size() > 2)
    {
        unsigned int counts = ScrollPoints->size() - 2;

        ScrollPeriod = 0;
        for(unsigned int i = 0; i < counts; i++)
        {
            CircularIncrement(FirstSpriteIndex, SpriteList);
        }
    }

    AllocateSpritePoints();
//todo: may want to handle this properly
//    CurrentScrollState = ScrollStatePageChange;
}

void ScrollingList::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();
    TweenEnterTime = 0;
    CurrentScrollDirection = ScrollDirectionIdle;
    RequestedScrollDirection = ScrollDirectionIdle;
    CurrentScrollState = ScrollStateIdle;
    ScrollAcceleration = 6;  // todo: make configurable
    ScrollPeriod = 0;

    for(unsigned int i = 0; SpriteList && i < SpriteList->size(); i++)
    {
        ComponentItemBinding *s = SpriteList->at(i);

        DeallocateTexture(s);
    }
}

void ScrollingList::TriggerMenuEnterEvent()
{
    Focus = true;

    if(!ScrollPoints) { return; }
    if(!SpriteList) { return; }
    if(SpriteList->size() == 0 ) { return; }
    if(FirstSpriteIndex >= SpriteList->size()) { return; }

    unsigned int spriteIndex = FirstSpriteIndex;

    for(unsigned int i = 0; i < ScrollPoints->size(); ++i)
    {
        ComponentItemBinding *s = SpriteList->at(spriteIndex);

        Component *c = s->GetComponent();
        if(c)
        {
            c->TriggerMenuEnterEvent();
        }

        CircularIncrement(spriteIndex, SpriteList);
    }
}

void ScrollingList::TriggerMenuExitEvent()
{
    Focus = false;
    if(!ScrollPoints) { return; }
    if(!SpriteList) { return; }
    if(SpriteList->size() == 0 ) { return; }
    if(FirstSpriteIndex >= SpriteList->size()) { return; }

    unsigned int spriteIndex = FirstSpriteIndex;

    for(unsigned int i = 0; i < ScrollPoints->size(); ++i)
    {
        ComponentItemBinding *s = SpriteList->at(spriteIndex);

        Component *c = s->GetComponent();
        if(c)
        {
            c->TriggerMenuExitEvent();
        }

        CircularIncrement(spriteIndex, SpriteList);
    }
}

void ScrollingList::Update(float dt)
{
    float scrollPeriod = 0;
    bool initializePoints = false;
    Component::Update(dt);

    if(!ScrollPoints) { return; }
    if(!SpriteList) { return; }
    if(SpriteList->size() == 0) { return; }

    bool readyToScroll = true;
    bool scrollChanged = false;
    bool scrollRequested = false;
    bool scrollStopped = false;

    // validate all scroll points are done tweening to the next position
    for(unsigned int i = 0; i < SpriteList->size(); i++)
    {
        ComponentItemBinding *s = SpriteList->at(i);
        Component *c = s->GetComponent();

        if(c && c->IsMenuScrolling())
        {
            readyToScroll = false;
            break;
        }
    }

    // check if it was requested to change directions
    if(CurrentScrollState == ScrollStateActive && RequestedScrollDirection != CurrentScrollDirection)
    {
        CurrentScrollState = ScrollStateStopping;
    }
    else if(CurrentScrollState == ScrollStateIdle && readyToScroll)
    {
        ScrollPeriod = 0.500;
        // check to see if requested to scroll
        if(RequestedScrollDirection != ScrollDirectionIdle)
        {
            initializePoints = true;
            CurrentScrollState = ScrollStateActive;
            CurrentScrollDirection = RequestedScrollDirection;
            scrollRequested = true;
        }
    }

    // if currently scrolling, process it
    if(!scrollRequested && readyToScroll)
    {
        if(CurrentScrollState == ScrollStateStopping)
        {
            Click(0);
            CurrentScrollState = ScrollStateIdle;
            scrollStopped = true;
            
            // update the tweens now that we are done
            unsigned int spriteIndex = FirstSpriteIndex;

            for(unsigned int i = 0; i < TweenPoints->size(); ++i)
            {
                ComponentItemBinding *s = SpriteList->at(spriteIndex);

                Component *c = s->GetComponent();
                if(c)
                {
                    c->SetTweens(TweenPoints->at(i));
                }

                CircularIncrement(spriteIndex, SpriteList);
            }

        }

        else if(CurrentScrollState == ScrollStateActive)
        {
            ScrollPeriod -= 0.050f;
            if(ScrollPeriod < 0.050)
            {
                ScrollPeriod = 0.050f;
            }

            Click(ScrollPeriod);
            scrollChanged = true;
        }
    }


    unsigned int spriteIndex = FirstSpriteIndex;
    for(unsigned int i = 0; i < ScrollPoints->size(); i++)
    {
        UpdateSprite(spriteIndex, i, (scrollRequested || scrollChanged), dt, ScrollPeriod);
        CircularIncrement(spriteIndex, SpriteList);
    }

    if(scrollStopped || scrollChanged)
    {
        ComponentItemBinding *sprite = GetPendingCollectionItemSprite();
        Item *item = NULL;

        if(sprite)
        {
            item = sprite->GetCollectionItem();
        }

        for(std::vector<MenuNotifierInterface *>::iterator it = NotificationComponents.begin();
                it != NotificationComponents.end();
                it++)
        {
            MenuNotifierInterface *c = *it;
            if(c && item)
            {
                c->OnNewItemSelected(item);
            }
        }

        if(CurrentScrollState == ScrollStatePageChange)
        {
            CurrentScrollState = ScrollStateIdle;
        }
    }
}

void ScrollingList::UpdateSprite(unsigned int spriteIndex, unsigned int pointIndex, bool newScroll, float dt, double nextScrollTime)
{
    ComponentItemBinding *s = SpriteList->at(spriteIndex);

    Component *c = s->GetComponent();
    //todo: remove me
    if(c && newScroll)
    {
        ViewInfo *currentViewInfo = ScrollPoints->at(pointIndex);
        unsigned int nextI = GetNextTween(pointIndex, ScrollPoints);
        ViewInfo *nextViewInfo = ScrollPoints->at(nextI);

        ResetTweens(c, TweenPoints->at(pointIndex), currentViewInfo, nextViewInfo, nextScrollTime);
        c->TriggerMenuScrollEvent();
    }
    if(c)
    {
        c->Update(dt);
    }

    CircularIncrement(spriteIndex, SpriteList);
}

void ScrollingList::ResetTweens(Component *c, TweenSet *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime)
{
    if(!c) { return; }
    if(!sets) { return; }
    if(!currentViewInfo) { return; }
    if(!nextViewInfo) { return; }

    currentViewInfo->SetImageHeight(c->GetBaseViewInfo()->GetImageHeight());
    currentViewInfo->SetImageWidth(c->GetBaseViewInfo()->GetImageWidth());
    nextViewInfo->SetImageHeight(c->GetBaseViewInfo()->GetImageHeight());
    nextViewInfo->SetImageWidth(c->GetBaseViewInfo()->GetImageWidth());
    nextViewInfo->SetBackgroundAlpha(c->GetBaseViewInfo()->GetBackgroundAlpha());

    //todo: delete properly, memory leak (big), proof of concept
    c->SetTweens(sets);

    TweenSets *scrollTween = sets->GetOnMenuScrollTweens();
    TweenSets::iterator it = scrollTween->begin();

    while(it != scrollTween->end())
    {
        std::vector<Tween *>::iterator it2 = (*it)->begin();
        while(it2 != (*it)->end())
        {
            delete *it2;
            (*it)->erase(it2);
            it2 = (*it)->begin();
        }
        delete *it;
        scrollTween->erase(it);
        it = scrollTween->begin();
        
    }
                    
    scrollTween->clear();
    c->UpdateBaseViewInfo(*currentViewInfo);

    std::vector<Tween *> *set = new std::vector<Tween *>();
    set->push_back(new Tween(TWEEN_PROPERTY_HEIGHT, LINEAR, currentViewInfo->GetHeight(), nextViewInfo->GetHeight(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_WIDTH, LINEAR, currentViewInfo->GetWidth(), nextViewInfo->GetWidth(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_ANGLE, LINEAR, currentViewInfo->GetAngle(), nextViewInfo->GetAngle(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_ALPHA, LINEAR, currentViewInfo->GetAlpha(), nextViewInfo->GetAlpha(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_X, LINEAR, currentViewInfo->GetX(), nextViewInfo->GetX(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_Y, LINEAR, currentViewInfo->GetY(), nextViewInfo->GetY(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_X_ORIGIN, LINEAR, currentViewInfo->GetXOrigin(), nextViewInfo->GetXOrigin(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_Y_ORIGIN, LINEAR, currentViewInfo->GetYOrigin(), nextViewInfo->GetYOrigin(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_X_OFFSET, LINEAR, currentViewInfo->GetXOffset(), nextViewInfo->GetXOffset(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_Y_OFFSET, LINEAR, currentViewInfo->GetYOffset(), nextViewInfo->GetYOffset(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_FONT_SIZE, LINEAR, currentViewInfo->GetFontSize(), nextViewInfo->GetFontSize(), scrollTime));
    set->push_back(new Tween(TWEEN_PROPERTY_BACKGROUND_ALPHA, LINEAR, currentViewInfo->GetBackgroundAlpha(), nextViewInfo->GetBackgroundAlpha(), scrollTime));
    scrollTween->push_back(set);
}


bool ScrollingList::AllocateTexture(ComponentItemBinding *s)
{
    
    if(!s || s->GetComponent() != NULL) { return false; }

    const Item *item = s->GetCollectionItem();
    //todo: will create a runtime fault if not of the right type
    //todo: remove coupling from knowing the collection name

    std::string videoKey ="collections." + GetCollectionName() + ".media.video";
    std::string imagePath;
    std::string videoPath;

    Component *t = NULL;

    ImageBuilder imageBuild;
    Config.GetMediaPropertyAbsolutePath(GetCollectionName(), ImageType, imagePath);
    t = imageBuild.CreateImage(imagePath, item->GetName(), ScaleX, ScaleY);

    if(!t && item->GetTitle() != item->GetFullTitle())
    {
        t = imageBuild.CreateImage(imagePath, item->GetFullTitle(), ScaleX, ScaleY);
    }
    if (!t)
    {
        t = new Text(item->GetTitle(), FontInst, FontColor, ScaleX, ScaleY);
    }

    if(t)
    {
        s->SetComponent(t);
    }

    return true;
}

void ScrollingList::DeallocateTexture(ComponentItemBinding *s)
{
    if(s && s->GetComponent() != NULL)
    {
        delete s->GetComponent();
        //todo: memory leak here, need to destroy allocated tween points here and in page (cannot be destroyed by component)
    }
    s->SetComponent(NULL);
}

void ScrollingList::Draw()
{
    //todo: Poor design implementation.
    // caller should instead call ScrollingList::Draw(unsigned int layer)
}

void ScrollingList::Draw(unsigned int layer)
{
    if(!ScrollPoints) { return; }
    if(!SpriteList) { return; }
    if(SpriteList->size() == 0) { return; }

    unsigned int spriteIndex = FirstSpriteIndex;

    for(unsigned int i = 0; i < ScrollPoints->size(); i++)
    {
        ComponentItemBinding *s = SpriteList->at(spriteIndex);
        Component *c = s->GetComponent();
        ViewInfo *currentViewInfo = ScrollPoints->at(i);

        if(c && currentViewInfo && currentViewInfo->GetLayer() == layer)
        {
            c->Draw();
        }
        CircularIncrement(spriteIndex, SpriteList);
    }
}


void ScrollingList::SetScrollDirection(ScrollDirection direction)
{
    RequestedScrollDirection = direction;

    ScrollStopRequested = (direction == ScrollDirectionIdle);
}

void ScrollingList::RemoveSelectedItem()
{
    ComponentItemBinding *sprite = GetSelectedCollectionItemSprite();
    if(sprite && SpriteList)
    {
        Item *item = sprite->GetCollectionItem();
        DeallocateTexture(sprite);
        int index = (FirstSpriteIndex + SelectedSpriteListIndex) % SpriteList->size();

        std::vector<ComponentItemBinding *>::iterator it = SpriteList->begin() + index;

        SpriteList->erase(it);
        delete sprite;

        if(item)
        {
            delete item;
        }

        if(SelectedSpriteListIndex >= SpriteList->size())
        {
            SelectedSpriteListIndex = 0;
        }
        if(FirstSpriteIndex >= SpriteList->size())
        {
            FirstSpriteIndex = 0;
        }
    }
}


std::vector<ComponentItemBinding *> *ScrollingList::GetCollectionItemSprites()
{
    return SpriteList;
}

ComponentItemBinding* ScrollingList::GetSelectedCollectionItemSprite()
{
    ComponentItemBinding *item = NULL;

    if(SpriteList && SpriteList->size() > 0)
    {
        int index = (FirstSpriteIndex + SelectedSpriteListIndex) % SpriteList->size();

        item = SpriteList->at(index);
    }

    return item;
}

ComponentItemBinding* ScrollingList::GetPendingCollectionItemSprite()
{
    ComponentItemBinding *item = NULL;
    unsigned int index = FirstSpriteIndex;
    if(SpriteList && SpriteList->size() > 0)
    {
        index = (index + SelectedSpriteListIndex) % SpriteList->size();

        item = SpriteList->at(index);
    }

    return item;
}

void ScrollingList::AddComponentForNotifications(MenuNotifierInterface *c)
{
    NotificationComponents.push_back(c);
}
void ScrollingList::RemoveComponentForNotifications(MenuNotifierInterface *c)
{
    for(std::vector<MenuNotifierInterface *>::iterator it = NotificationComponents.begin();
            it != NotificationComponents.end();
            it++)
    {
        if(c == *it)
        {
            NotificationComponents.erase(it);
            break;
        }
    }
}


ComponentItemBinding* ScrollingList::GetPendingSelectedCollectionItemSprite()
{
    ComponentItemBinding *item = NULL;

    if(SpriteList)
    {
        unsigned int index = SelectedSpriteListIndex;

        if(CurrentScrollDirection == ScrollDirectionBack)
        {
            CircularDecrement(index, SpriteList);
        }
        if(CurrentScrollDirection == ScrollDirectionForward)
        {
            CircularIncrement(index, SpriteList);
        }

        if(SpriteList && SpriteList->size() > 0)
        {
            index = (index + SelectedSpriteListIndex) % SpriteList->size();

            item = SpriteList->at(index);
        }
    }

    return item;
}

bool ScrollingList::IsIdle()
{
    return (Component::IsIdle() && CurrentScrollState == ScrollStateIdle);
}

void ScrollingList::CircularIncrement(unsigned int &index, std::vector<ViewInfo*>* list)
{
    index++;

    if(index >= list->size())
    {
        index = 0;
    }
}

void ScrollingList::CircularDecrement(unsigned int &index, std::vector<ViewInfo*>* list)
{
    if(index > 0)
    {
        index--;
    }
    else
    {
        if(list->size() > 0)
        {
            index = list->size() - 1;
        }
        else
        {
            index = 0;
        }
    }
}

int ScrollingList::CircularIncrement(unsigned int index, unsigned int offset, std::vector<ComponentItemBinding *> *list)
{
    unsigned int end = index + offset;

    while(end >= list->size() && list->size() > 0)
    {
         end -= list->size();
    }

    return end;
}


void ScrollingList::CircularIncrement(unsigned int &index, std::vector<ComponentItemBinding*> *list)
{
    index++;

    if(index >= list->size())
    {
        index = 0;
    }
}

void ScrollingList::CircularDecrement(unsigned int &index, std::vector<ComponentItemBinding*> *list)
{
    if(index > 0)
    {
        index--;
    }
    else
    {
        if(list && list->size() > 0)
        {
            index = list->size() - 1;
        }
        else
        {
            index = 0;
        }
    }
}

