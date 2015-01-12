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
                             std::string collectionName,
                             std::string imageType)
    : IsScrollChangedStarted(true)
    , IsScrollChangedSignalled(false)
    , IsScrollChangedComplete(false)
    , SpriteList(NULL)
    , ScrollPoints(NULL)
    , TweenEnterTime(0)
    , FirstSpriteIndex(0)
    , SelectedSpriteListIndex(0)
    , CurrentAnimateTime(0) // in seconds
    , ScrollTime(0)         // in seconds
    , CurrentScrollDirection(ScrollDirectionIdle)
    , RequestedScrollDirection(ScrollDirectionIdle)
    , CurrentScrollState(ScrollStateIdle)
    , ScrollAcceleration(6)  // todo: make configurable
    , ScrollVelocity(0)
    , Config(c)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
    , FontInst(font)
    , FontColor(fontColor)
    , LayoutKey(layoutKey)
    , CollectionName(collectionName)
    , ImageType(imageType)
    , MaxLayer(0)
{
}

ScrollingList::~ScrollingList()
{
    if(SpriteList)
    {
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
}

void ScrollingList::SetItems(std::vector<ComponentItemBinding *> *spriteList)
{
    SpriteList = spriteList;
    FirstSpriteIndex = 0;

    // loop the scroll points if there are not enough

    if(SpriteList)
    {
        unsigned int originalSize = SpriteList->size();

        while(ScrollPoints && ScrollPoints->size()+4 > SpriteList->size() && SpriteList->size() > 0)
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

        IsScrollChangedComplete = true;
    }
}

void ScrollingList::SetPoints(std::vector<ViewInfo *> *scrollPoints)
{
    ScrollPoints = scrollPoints;
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

void ScrollingList::Click()
{
    if(CurrentScrollDirection == ScrollDirectionBack)
    {
        CircularDecrement(FirstSpriteIndex, SpriteList);
        IsScrollChangedComplete = true;
    }
    if(CurrentScrollDirection == ScrollDirectionForward)
    {
        CircularIncrement(FirstSpriteIndex, SpriteList);
        IsScrollChangedComplete = true;
    }
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
    if(ScrollPoints && ScrollPoints->size() > 4)
    {
        ScrollVelocity = 0;
        unsigned int counts = ScrollPoints->size() - 4;

        for(unsigned int i = 0; i < counts; i++)
        {
            CircularDecrement(FirstSpriteIndex, SpriteList);
        }
    }

    CurrentScrollState = ScrollStatePageChange;
    IsScrollChangedStarted = true;
    IsScrollChangedSignalled = false;
    IsScrollChangedComplete = false;
}

void ScrollingList::PageDown()
{
    if(ScrollPoints && ScrollPoints->size() > 4)
    {
        unsigned int counts = ScrollPoints->size() - 4;

        ScrollVelocity = 0;
        for(unsigned int i = 0; i < counts; i++)
        {
            CircularIncrement(FirstSpriteIndex, SpriteList);
        }
    }

    CurrentScrollState = ScrollStatePageChange;
    IsScrollChangedStarted = true;
    IsScrollChangedSignalled = false;
    IsScrollChangedComplete = false;
}

void ScrollingList::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();
    TweenEnterTime = 0;
    CurrentAnimateTime = 0;
    ScrollTime = 0;
    CurrentScrollDirection = ScrollDirectionIdle;
    RequestedScrollDirection = ScrollDirectionIdle;
    CurrentScrollState = ScrollStateIdle;
    ScrollAcceleration = 6;  // todo: make configurable
    ScrollVelocity = 0;

    for(unsigned int i = 0; i < SpriteList->size(); i++)
    {
        ComponentItemBinding *s = SpriteList->at(i);

        DeallocateTexture(s);
    }
}


void ScrollingList::Update(float dt)
{
    float scrollPeriod = 0;
    Component::Update(dt);

    if(!ScrollPoints)
    {
        return;
    }

    switch(CurrentScrollState)
    {
    case ScrollStateActive:
        if(RequestedScrollDirection != CurrentScrollDirection)
        {
            CurrentScrollState = ScrollStateStopping;

        }

        break;

    case ScrollStateIdle:
        ScrollTime = 0;
        CurrentAnimateTime = 0;
        ScrollVelocity = 0;

        if(RequestedScrollDirection != ScrollDirectionIdle)
        {
            CurrentScrollState = ScrollStateActive;
            CurrentScrollDirection = RequestedScrollDirection;
        }
        break;
    default:
        break;

    };

    if(CurrentScrollState != ScrollStatePageChange && CurrentScrollState != ScrollStateIdle)
    {
        IsScrollChangedStarted = true;
        ScrollTime += dt;
        CurrentAnimateTime += dt;
        ScrollVelocity = ScrollTime * ScrollAcceleration;

        // clip at 5 items scrolled per second
        if(ScrollVelocity > 30)
        {
            ScrollVelocity = 30;
        }
        if(ScrollVelocity > 0)
        {
            scrollPeriod = 1 / ScrollVelocity;
        }

        // have we exceeded the time of when to stop on the next item in the list?
        if(CurrentScrollState == ScrollStateStopping && CurrentAnimateTime >= scrollPeriod)
        {
            Click();
            CurrentAnimateTime = 0;
            ScrollVelocity = 0;
            CurrentScrollState = ScrollStateIdle;
        }
    }

    while(CurrentScrollState != ScrollStatePageChange && ScrollVelocity > 0 && CurrentAnimateTime >= scrollPeriod)
    {
        Click();
        CurrentAnimateTime -= scrollPeriod;
    }


    if(ScrollPoints && SpriteList->size() > 0 && FirstSpriteIndex < SpriteList->size())
    {
        unsigned int spriteIndex = FirstSpriteIndex;
        unsigned int numIterations = (ScrollPoints->size() > SpriteList->size()) ? SpriteList->size() : ScrollPoints->size();
        unsigned int start = (ScrollPoints->size() > SpriteList->size()) ? SelectedSpriteListIndex : 0;

        for(unsigned int i = start; i < start+numIterations && spriteIndex < SpriteList->size(); i++)
        {
            ComponentItemBinding *s = SpriteList->at(spriteIndex);
            unsigned int nextI = GetNextTween(i, ScrollPoints);

            ViewInfo *currentViewInfo = ScrollPoints->at(i);
            ViewInfo *nextViewInfo = ScrollPoints->at(nextI);

            AllocateTexture(s);

            Component *c = s->GetComponent();
            if(c)
            {
                currentViewInfo->SetImageHeight(c->GetBaseViewInfo()->GetImageHeight());
                currentViewInfo->SetImageWidth(c->GetBaseViewInfo()->GetImageWidth());
                nextViewInfo->SetImageHeight(c->GetBaseViewInfo()->GetImageHeight());
                nextViewInfo->SetImageWidth(c->GetBaseViewInfo()->GetImageWidth());

                //todo: 30 is a magic number
                ViewInfo *spriteViewInfo = c->GetBaseViewInfo();

                spriteViewInfo->SetX(Tween::AnimateSingle(LINEAR, currentViewInfo->GetX(), nextViewInfo->GetX(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetY(Tween::AnimateSingle(LINEAR, currentViewInfo->GetY(), nextViewInfo->GetY(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetXOrigin(Tween::AnimateSingle(LINEAR, currentViewInfo->GetXOrigin(), nextViewInfo->GetXOrigin(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetYOrigin(Tween::AnimateSingle(LINEAR, currentViewInfo->GetYOrigin(), nextViewInfo->GetYOrigin(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetXOffset(Tween::AnimateSingle(LINEAR, currentViewInfo->GetXOffset(), nextViewInfo->GetXOffset(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetYOffset(Tween::AnimateSingle(LINEAR, currentViewInfo->GetYOffset(), nextViewInfo->GetYOffset(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetHeight(Tween::AnimateSingle(LINEAR, currentViewInfo->GetHeight(), nextViewInfo->GetHeight(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetWidth(Tween::AnimateSingle(LINEAR, currentViewInfo->GetWidth(), nextViewInfo->GetWidth(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetAlpha(Tween::AnimateSingle(LINEAR, currentViewInfo->GetAlpha(), nextViewInfo->GetAlpha(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetAngle(Tween::AnimateSingle(LINEAR, currentViewInfo->GetAngle(), nextViewInfo->GetAngle(), scrollPeriod, CurrentAnimateTime));
                spriteViewInfo->SetFontSize(Tween::AnimateSingle(LINEAR, currentViewInfo->GetFontSize(), nextViewInfo->GetFontSize(), scrollPeriod, CurrentAnimateTime));
                c->Update(dt);

            }

            CircularIncrement(spriteIndex, SpriteList);
        }

        // start freeing up memory if the list is too large
        if(SpriteList->size() + 4 > ScrollPoints->size())
        {
            spriteIndex = FirstSpriteIndex;

            CircularDecrement(spriteIndex, SpriteList);
            DeallocateTexture(SpriteList->at(spriteIndex));

            CircularDecrement(spriteIndex, SpriteList);
            DeallocateTexture(SpriteList->at(spriteIndex));


            // point to the end of the list to start deallocating..
            // It's not fast, but it's easy to read
            spriteIndex = FirstSpriteIndex;
            for(unsigned int i = 0; i < ScrollPoints->size(); i++)
            {
                CircularIncrement(spriteIndex, SpriteList);
            }

            CircularIncrement(spriteIndex, SpriteList);
            DeallocateTexture(SpriteList->at(spriteIndex));

            CircularIncrement(spriteIndex, SpriteList);
            DeallocateTexture(SpriteList->at(spriteIndex));
        }
    }

    if(IsScrollChangedStarted && !IsScrollChangedSignalled)
    {
        IsScrollChangedSignalled = true;
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
            IsScrollChangedComplete = true;
            CurrentScrollState = ScrollStateIdle;
        }
    }

    if(IsScrollChangedStarted && IsScrollChangedSignalled && IsScrollChangedComplete)
    {
        IsScrollChangedStarted = false;
        IsScrollChangedComplete = false;
        IsScrollChangedSignalled = false;
    }

}

void ScrollingList::AllocateTexture(ComponentItemBinding *s)
{
    //todo: move this outside of the Draw routine
    if(s && s->GetComponent() == NULL)
    {
        const Item *item = s->GetCollectionItem();
        //todo: will create a runtime fault if not of the right type
        //todo: remove coupling from knowing the collection name

        std::string collectionKey ="collections." + CollectionName + ".media." + ImageType;
        std::string videoKey ="collections." + CollectionName + ".media.video";
        std::string imagePath;
        std::string videoPath;

        Component *t = NULL;

        /*
         // todo: to be supported at a later date
        if(c->GetProperty(videoKey, videoPath))
        {
           t = new VideoComponent(videoPath, item->GetFullTitle(), ScaleX, ScaleY);
        }
        */
        if(!t && Config.GetPropertyAbsolutePath(collectionKey, imagePath))
        {
            ImageBuilder imageBuild;
            t = imageBuild.CreateImage(imagePath, item->GetName(), ScaleX, ScaleY);

            if(!t && item->GetTitle() != item->GetFullTitle())
            {
                t = imageBuild.CreateImage(imagePath, item->GetFullTitle(), ScaleX, ScaleY);
            }
        }
        if (!t)
        {
            t = new Text(item->GetTitle(), FontInst, FontColor, ScaleX, ScaleY);
        }

        if(t)
        {
            s->SetComponent(t);
        }
    }

}

void ScrollingList::DeallocateTexture(ComponentItemBinding *s)
{
    if(s && s->GetComponent() != NULL)
    {
        delete s->GetComponent();
    }
    s->SetComponent(NULL);
}

void ScrollingList::Draw()
{
    //todo: Poor design implementation.
    // caller should instead call ScrollingList::Draw(unsigned int layer)
}

//todo: this is kind of a hack. Aggregation needs to happen differently
void ScrollingList::Draw(unsigned int layer)
{
    if(ScrollPoints && SpriteList && SpriteList->size() > 0 && FirstSpriteIndex < SpriteList->size())
    {
        unsigned int spriteIndex = FirstSpriteIndex;

        for(unsigned int i = 0; i < ScrollPoints->size(); i++)
        {
            std::vector<ComponentItemBinding *>::iterator it = SpriteList->begin() + spriteIndex;
            Component *c = (*it)->GetComponent();
            ViewInfo *currentViewInfo = ScrollPoints->at(i);

            if(currentViewInfo && currentViewInfo->GetLayer() == layer)
            {
                c->Draw();
            }
            CircularIncrement(spriteIndex, SpriteList);
        }
    }
}


void ScrollingList::SetScrollDirection(ScrollDirection direction)
{
    RequestedScrollDirection = direction;
}

void ScrollingList::RemoveSelectedItem()
{
    ComponentItemBinding *sprite = GetSelectedCollectionItemSprite();
    if(sprite)
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
    IsScrollChangedComplete = true;
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
    if(CurrentScrollState != ScrollStatePageChange)
    {
        if(CurrentScrollDirection == ScrollDirectionBack)
        {
            CircularDecrement(index, SpriteList);
        }
        if(CurrentScrollDirection == ScrollDirectionForward)
        {
            CircularIncrement(index, SpriteList);
        }
    }
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

