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
#include "../Animate/TweenSet.h"
#include "../Animate/Animation.h"
#include "../Animate/AnimationEvents.h"
#include "../Animate/TweenTypes.h"
#include "../Font.h"
#include "ScrollingList.h"
#include "ImageBuilder.h"
#include "VideoBuilder.h"
#include "VideoComponent.h"
#include "ReloadableMedia.h"
#include "Text.h"
#include "../../Database/Configuration.h" // todo: decouple the GUI from the data
#include "../../Collection/Item.h"
#include "../../Utility/Utils.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include "../ViewInfo.h"
#include <math.h>
#include <SDL2/SDL_image.h>
#include <sstream>
#include <cctype>


//todo: remove coupling from configuration data (if possible)
ScrollingList::ScrollingList(Configuration &c,
                             float scaleX,
                             float scaleY,
                             Font *font,
                             std::string layoutKey,
                             std::string imageType)
    : horizontalScroll(false)
    , spriteList_(NULL)
    , scrollPoints_(NULL)
    , tweenPoints_(NULL)
    , focus_(false)
    , itemIndex_(0)
    , componentIndex_(0)
    , scrollStopRequested_(true)
    , notifyAllRequested_(false)
    , currentScrollDirection_(ScrollDirectionIdle)
    , requestedScrollDirection_(ScrollDirectionIdle)
    , currentScrollState_(ScrollStateIdle)
    , scrollAcceleration_(0)
    , startScrollTime_(0.500)
    , scrollPeriod_(0)
    , config_(c)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
    , fontInst_(font)
    , layoutKey_(layoutKey)
    , imageType_(imageType)
    , items_(NULL)
{
}

ScrollingList::ScrollingList(const ScrollingList &copy)
    : Component(copy)
    , horizontalScroll(copy.horizontalScroll)
    , spriteList_(NULL)
    , focus_(false)
    , itemIndex_(0)
    , componentIndex_(0)
    , scrollStopRequested_(true)
    , notifyAllRequested_(false)
    , currentScrollDirection_(ScrollDirectionIdle)
    , requestedScrollDirection_(ScrollDirectionIdle)
    , currentScrollState_(ScrollStateIdle)
    , scrollAcceleration_(copy.scrollAcceleration_)
    , startScrollTime_(copy.startScrollTime_)
    , scrollPeriod_(0)
    , config_(copy.config_)
    , scaleX_(copy.scaleX_)
    , scaleY_(copy.scaleY_)
    , fontInst_(copy.fontInst_)
    , layoutKey_(copy.layoutKey_)
    , imageType_(copy.imageType_)
    , items_(NULL)
{
    scrollPoints_ = NULL;
    tweenPoints_ = NULL;

    if(copy.scrollPoints_)
    {
        scrollPoints_ = new std::vector<ViewInfo *>();
        for(unsigned int i = 0; i < copy.scrollPoints_->size(); ++i)
        {
            ViewInfo *v = new ViewInfo(*copy.scrollPoints_->at(i));
            scrollPoints_->push_back(v);
        }
    }

    if(copy.tweenPoints_)
    {
        tweenPoints_ = new std::vector<AnimationEvents *>();
        for(unsigned int i = 0; i < copy.tweenPoints_->size(); ++i)
        {
            AnimationEvents *v = new AnimationEvents(*copy.tweenPoints_->at(i));
            tweenPoints_->push_back(v);
        }
    }
}


ScrollingList::~ScrollingList()
{
    destroyItems();
}


void ScrollingList::setItems(CollectionInfo *info)
{
    deallocateSpritePoints();

    collection_ = info;
    items_ = &collection_->items;

    itemIndex_ = 0;
    componentIndex_ = 0;
  
    allocateSpritePoints();

    notifyAllRequested_ = true;

}

unsigned int ScrollingList::loopIncrement(unsigned int offset, unsigned int i, unsigned int size)
{
    if(size == 0) return 0;
    return (offset + i) % size;
}

unsigned int ScrollingList::loopDecrement(unsigned int offset, unsigned int i, unsigned int size)
{
    if(size == 0) return 0;
   return ((offset % size) - (i % size) + size) % size; 
}


void ScrollingList::setScrollAcceleration(float value)
{
    scrollAcceleration_ = value;
}

void ScrollingList::setStartScrollTime(float value)
{
    startScrollTime_ = value;
}

void ScrollingList::deallocateSpritePoints()
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        deallocateTexture(i);
    }

    componentIndex_ = 0;
}

void ScrollingList::allocateSpritePoints()
{
    for(unsigned int i = 0; items_ && i < scrollPoints_->size(); ++i)
    {
        componentIndex_ = 0;
        unsigned int index = loopIncrement(itemIndex_, i, items_->size());
        Item *item = items_->at(index);

        allocateTexture(i, item);
        Component *c = components_.at(i);

        ViewInfo *current = scrollPoints_->at(i);

        unsigned int nextI = loopIncrement(i, 1, scrollPoints_->size());
        ViewInfo *next = scrollPoints_->at(nextI);

        resetTweens(c, tweenPoints_->at(i), current, next, 0);
    }
}

void ScrollingList::destroyItems()
{
    deallocateSpritePoints();  
//todo: who deletes the CollectionInfo?
}


void ScrollingList::setPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<AnimationEvents *> *tweenPoints)
{
    deallocateSpritePoints();

    scrollPoints_ = scrollPoints;
    tweenPoints_ = tweenPoints;

    // empty out the list as we will resize it
    components_.clear();

    if(scrollPoints && scrollPoints_->size() > components_.size())
    {
        components_.resize(scrollPoints_->size(), NULL);
    }
    allocateSpritePoints();
}

unsigned int ScrollingList::getScrollOffsetIndex()
{
    return itemIndex_;
}

void ScrollingList::setScrollOffsetIndex(unsigned int index)
{
    itemIndex_ = index;
}

void ScrollingList::setSelectedIndex(int selectedIndex)
{
    selectedOffsetIndex_ = selectedIndex;
}

void ScrollingList::click(double nextScrollTime)
{
    if(currentScrollDirection_ == ScrollDirectionBack)
    {
        // get the previous item
        itemIndex_ = loopDecrement(itemIndex_, 1, items_->size());
        Item *i = items_->at(itemIndex_);
        componentIndex_ = loopDecrement(componentIndex_, 1, components_.size()); 

        deallocateTexture(componentIndex_);
        allocateTexture(componentIndex_, i);
    }
    else if(currentScrollDirection_ == ScrollDirectionForward)
    {
        itemIndex_ = loopIncrement(itemIndex_, 1, items_->size());
        Item *i = items_->at(itemIndex_);
       

        deallocateTexture(componentIndex_);
        allocateTexture(componentIndex_, i);

        componentIndex_ = loopIncrement(componentIndex_, 1, components_.size()); 
    }
}

void ScrollingList::pageUp()
{
    notifyAllRequested_ = true;

    if(components_.size() == 0) return;

    deallocateSpritePoints();

    itemIndex_ = loopDecrement(itemIndex_, components_.size(), items_->size());
    
    allocateSpritePoints();
}

void ScrollingList::pageDown()
{
    notifyAllRequested_ = true;

    if(components_.size() == 0) return;

    deallocateSpritePoints();

    itemIndex_ = loopIncrement(itemIndex_, components_.size(), items_->size());

    allocateSpritePoints();
}


void ScrollingList::letterUp()
{
    letterChange(true);
}

void ScrollingList::letterDown()
{
    letterChange(false);
}

void ScrollingList::letterChange(bool increment)
{
    notifyAllRequested_ = true;
    deallocateSpritePoints();

    std::string startname = items_->at(itemIndex_)->lowercaseFullTitle();

    for(unsigned int i = 0; i < items_->size(); ++i)
    {
        unsigned int index = 0;
        if(increment) index = loopIncrement(itemIndex_, i, items_->size());
        else index = loopDecrement(itemIndex_, i, items_->size());

        std::string endname = items_->at(index)->lowercaseFullTitle();

        // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
        if(isalpha(startname[0]) ^ isalpha(endname[0]))
        {
           break; 
           itemIndex_ = index;
        }
        else if(isalpha(startname[0]) && isalpha(endname[0]) && startname[0] != endname[0])
        {
            itemIndex_ = index;
            break;
        }
    }

    allocateSpritePoints();
}


void ScrollingList::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
    currentScrollDirection_ = ScrollDirectionIdle;
    requestedScrollDirection_ = ScrollDirectionIdle;
    currentScrollState_ = ScrollStateIdle;
    scrollPeriod_ = 0;
    
    deallocateSpritePoints();
}

void ScrollingList::triggerMenuEnterEvent()
{
    focus_ = true;
    notifyAllRequested_ = true;

    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerMenuEnterEvent();
    }
}

void ScrollingList::triggerMenuExitEvent()
{
    focus_ = false;
    notifyAllRequested_ = true;

    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerMenuExitEvent();
    }
}

void ScrollingList::update(float dt)
{
    Component::update(dt);

    bool readyToScroll = true;
    bool scrollChanged = false;
    bool scrollRequested = false;
    bool scrollStopped = false;

    // validate all scroll points are done tweening to the next position
    for(unsigned int i = 0; i < components_.size(); i++)
    {
        Component *c = components_.at(i);

        if(c && c->isMenuScrolling())
        {
            readyToScroll = false;
            break;
        }
    }

    // check if it was requested to change directions
    if(currentScrollState_ == ScrollStateActive && requestedScrollDirection_ != currentScrollDirection_)
    {
        currentScrollState_ = ScrollStateStopping;
    }
    else if(currentScrollState_ == ScrollStateIdle && readyToScroll)
    {
        scrollPeriod_ = startScrollTime_;
        // check to see if requested to scroll
        if(requestedScrollDirection_ != ScrollDirectionIdle)
        {
            currentScrollState_ = ScrollStateActive;
            currentScrollDirection_ = requestedScrollDirection_;
            scrollRequested = true;
        }
    }

    // if currently scrolling, process it
    if(!scrollRequested && readyToScroll)
    {
        if(currentScrollState_ == ScrollStateStopping)
        {
            currentScrollState_ = ScrollStateIdle;
            scrollStopped = true;
            click(0);

            for(unsigned int i = 0; i < tweenPoints_->size(); ++i)
            {
                unsigned int cindex = loopIncrement(componentIndex_, i, components_.size());
                Component *c = components_.at(cindex);

                if(c) c->setTweens(tweenPoints_->at(i));
            }

        }

        else if(currentScrollState_ == ScrollStateActive)
        {
            scrollPeriod_ -= scrollAcceleration_;
            if(scrollPeriod_ < scrollAcceleration_)
            {
                scrollPeriod_ = scrollAcceleration_;
            }

            click(scrollPeriod_);
            scrollChanged = true;
        }
    }


    for(unsigned int i = 0; i < scrollPoints_->size(); i++)
    {
        unsigned int cindex = loopIncrement(componentIndex_, i, components_.size());

        Component *c = components_.at(cindex);

        if(c && (scrollRequested || scrollChanged))
        {
            unsigned int nextI = 0;
            if(currentScrollDirection_ == ScrollDirectionBack)
            {
                nextI = loopIncrement(i, 1, scrollPoints_->size());
            }
            if(currentScrollDirection_ == ScrollDirectionForward)
            {
                nextI = loopDecrement(i, 1, scrollPoints_->size());
            }

            ViewInfo *currentvi = scrollPoints_->at(i);
            ViewInfo *nextvi = scrollPoints_->at(nextI);

            resetTweens(c, tweenPoints_->at(i), currentvi, nextvi, scrollPeriod_);
            c->triggerMenuScrollEvent();
        }

        if(c) c->update(dt);
        
    }

    if(scrollStopped || (notifyAllRequested_ && focus_))
    {
        Item *item = NULL;
        unsigned index = loopIncrement(itemIndex_, selectedOffsetIndex_, items_->size());
        item = items_->at(index);

        for(std::vector<MenuNotifierInterface *>::iterator it = notificationComponents_.begin();
                it != notificationComponents_.end();
                it++)
        {
            MenuNotifierInterface *c = *it;

            if(c && item)
            {
                c->onNewItemSelected(item);
            }
        }

        if(currentScrollState_ == ScrollStatePageChange)
        {
            currentScrollState_ = ScrollStateIdle;
        }
    }

    notifyAllRequested_ = false;
}

void ScrollingList::resetTweens(Component *c, AnimationEvents *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime)
{
    if(!c) return;
    if(!sets) return;
    if(!currentViewInfo) return;
    if(!nextViewInfo) return;

    currentViewInfo->ImageHeight = c->baseViewInfo.ImageHeight;
    currentViewInfo->ImageWidth = c->baseViewInfo.ImageWidth;
    nextViewInfo->ImageHeight = c->baseViewInfo.ImageHeight;
    nextViewInfo->ImageWidth = c->baseViewInfo.ImageWidth;
    nextViewInfo->BackgroundAlpha = c->baseViewInfo.BackgroundAlpha;

    //todo: delete properly, memory leak (big), proof of concept
    c->setTweens(sets);

    Animation *scrollTween = sets->getAnimation("menuScroll");
    scrollTween->Clear();
    c->baseViewInfo = *currentViewInfo;

    TweenSet *set = new TweenSet();
    set->push(new Tween(TWEEN_PROPERTY_HEIGHT, EASE_INOUT_QUADRATIC, currentViewInfo->ScaledHeight(), nextViewInfo->ScaledHeight(), scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_WIDTH, EASE_INOUT_QUADRATIC, currentViewInfo->ScaledWidth(), nextViewInfo->ScaledWidth(), scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_ANGLE, EASE_INOUT_QUADRATIC, currentViewInfo->Angle, nextViewInfo->Angle, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_ALPHA, EASE_INOUT_QUADRATIC, currentViewInfo->Alpha, nextViewInfo->Alpha, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_X, EASE_INOUT_QUADRATIC, currentViewInfo->X, nextViewInfo->X, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_Y, EASE_INOUT_QUADRATIC, currentViewInfo->Y, nextViewInfo->Y, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_X_ORIGIN, EASE_INOUT_QUADRATIC, currentViewInfo->XOrigin, nextViewInfo->XOrigin, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_Y_ORIGIN, EASE_INOUT_QUADRATIC, currentViewInfo->YOrigin, nextViewInfo->YOrigin, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_X_OFFSET, EASE_INOUT_QUADRATIC, currentViewInfo->XOffset, nextViewInfo->XOffset, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_Y_OFFSET, EASE_INOUT_QUADRATIC, currentViewInfo->YOffset, nextViewInfo->YOffset, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_FONT_SIZE, EASE_INOUT_QUADRATIC, currentViewInfo->FontSize, nextViewInfo->FontSize, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_BACKGROUND_ALPHA, EASE_INOUT_QUADRATIC, currentViewInfo->BackgroundAlpha, nextViewInfo->BackgroundAlpha, scrollTime));
    scrollTween->Push(set);
}


bool ScrollingList::allocateTexture(unsigned int index, Item *item)
{

    if(index >= components_.size()) return false;

    //todo: will create a runtime fault if not of the right type
    //todo: remove coupling from knowing the collection name

    std::string videoKey ="collections." + collectionName + ".media.video";
    std::string imagePath;
    std::string videoPath;

    Component *t = NULL;

    ImageBuilder imageBuild;

    // check collection path for art based on gamename
    config_.getMediaPropertyAbsolutePath(collectionName, imageType_, false, imagePath);
    t = imageBuild.CreateImage(imagePath, item->name, scaleX_, scaleY_);

    // check sub-collection path for art based on gamename
    if(!t)
    {
        config_.getMediaPropertyAbsolutePath(item->collectionInfo->name, imageType_, false, imagePath);
        t = imageBuild.CreateImage(imagePath, item->name, scaleX_, scaleY_);
    }

    // check collection path for art based on game name (full title)
    if(!t && item->title != item->fullTitle)
    {
        config_.getMediaPropertyAbsolutePath(collectionName, imageType_, false, imagePath);
        t = imageBuild.CreateImage(imagePath, item->fullTitle, scaleX_, scaleY_);
    }

    // check sub-collection path for art based on game name (full title)
    if(!t && item->title != item->fullTitle)
    {
        config_.getMediaPropertyAbsolutePath(item->collectionInfo->name, imageType_, false, imagePath);
        t = imageBuild.CreateImage(imagePath, item->fullTitle, scaleX_, scaleY_);
    }

    // check collection path for art based on parent game name
    if(!t && item->cloneof != "")
    {
        config_.getMediaPropertyAbsolutePath(collectionName, imageType_, false, imagePath);
        t = imageBuild.CreateImage(imagePath, item->cloneof, scaleX_, scaleY_);
    }

    // check sub-collection path for art based on parent game name
    if(!t && item->cloneof != "")
    {
        config_.getMediaPropertyAbsolutePath(item->collectionInfo->name, imageType_, false, imagePath);
        t = imageBuild.CreateImage(imagePath, item->cloneof, scaleX_, scaleY_);
    }

    // check collection path for art based on system name
    if(!t)
    {
		config_.getMediaPropertyAbsolutePath(item->name, imageType_, true, imagePath);
		t = imageBuild.CreateImage(imagePath, imageType_, scaleX_, scaleY_);
    }

    if (!t)
    {
        t = new Text(item->title, fontInst_, scaleX_, scaleY_);
    }

    if(t)
    {
        components_.at(index) = t;
    }

    return true;
}

void ScrollingList::deallocateTexture(unsigned int index)
{
    if(components_.size() <= index) return;

    Component *s = components_.at(index);

    if(s)
    {
        delete s;
        components_.at(index) = NULL;
    }
}

void ScrollingList::draw()
{
    //todo: Poor design implementation.
    // caller should instead call ScrollingList::Draw(unsigned int layer)
}

void ScrollingList::draw(unsigned int layer)
{
    
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->draw();
    }
}


void ScrollingList::setScrollDirection(ScrollDirection direction)
{
    requestedScrollDirection_ = direction;

    scrollStopRequested_ = (direction == ScrollDirectionIdle);
}


void ScrollingList::addComponentForNotifications(MenuNotifierInterface *c)
{
    notificationComponents_.push_back(c);
}
void ScrollingList::removeComponentForNotifications(MenuNotifierInterface *c)
{
    for(std::vector<MenuNotifierInterface *>::iterator it = notificationComponents_.begin();
            it != notificationComponents_.end();
            it++)
    {
        if(c == *it)
        {
            notificationComponents_.erase(it);
            break;
        }
    }
}

bool ScrollingList::isIdle()
{
    return (Component::isIdle() && currentScrollState_ == ScrollStateIdle);
}


