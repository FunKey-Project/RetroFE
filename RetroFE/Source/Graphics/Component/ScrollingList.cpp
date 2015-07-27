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
    , tweenEnterTime_(0)
    , focus_(false)
    , firstSpriteIndex_(0)
    , selectedSpriteListIndex_(0)
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
{
}

ScrollingList::ScrollingList(const ScrollingList &copy)
    : Component(copy)
    , horizontalScroll(copy.horizontalScroll)
    , spriteList_(NULL)
    , tweenEnterTime_(0)
    , focus_(false)
    , firstSpriteIndex_(0)
    , selectedSpriteListIndex_(copy.selectedSpriteListIndex_)
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

void ScrollingList::setItems(std::vector<ComponentItemBinding *> *spriteList)
{
    notifyAllRequested_ = true;
    spriteList_ = spriteList;
    firstSpriteIndex_ = 0;

    if(!spriteList_)
    {
        return;
    }
    unsigned int originalSize = spriteList_->size();

    // loop the scroll points if there are not enough, the +2 represents the head and tail nodes (for when the item is allocated)
    while(scrollPoints_ && scrollPoints_->size()+2 > spriteList_->size() && spriteList_->size() > 0)
    {
        for(unsigned int i = 0; i < originalSize; ++i)
        {
            Item *newItem = new Item();
            Item *originalItem = spriteList_->at(i)->item;

            *newItem = *originalItem;
            ComponentItemBinding *newSprite = new ComponentItemBinding(newItem);
            spriteList_->push_back(newSprite);
        }
    }

    for(unsigned int i = 0; scrollPoints_ && i < selectedSpriteListIndex_; ++i)
    {
        circularDecrement(firstSpriteIndex_, spriteList_);
    }

    allocateSpritePoints();
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
    if(!spriteList_)
    {
        return;
    }

    unsigned int spriteIndex = firstSpriteIndex_;

    for(unsigned int i = 0; i < scrollPoints_->size() && spriteList_->size() > spriteIndex; ++i)
    {
        deallocateTexture(spriteList_->at(spriteIndex));
        circularIncrement(spriteIndex, spriteList_);
    }
}

void ScrollingList::allocateSpritePoints()
{
    if(!scrollPoints_)
    {
        return;
    }
    if(!spriteList_)
    {
        return;
    }
    if(spriteList_->size() == 0)
    {
        return;
    }
    if(!tweenPoints_)
    {
        return;
    }

    unsigned int spriteIndex = firstSpriteIndex_;

    for(unsigned int i = 0; i < scrollPoints_->size(); ++i)
    {
        allocateTexture(spriteList_->at(spriteIndex));
        Component *c = spriteList_->at(spriteIndex)->component;
        ViewInfo *currentViewInfo = scrollPoints_->at(i);
        unsigned int nextI = getNextTween(i, scrollPoints_);
        ViewInfo *nextViewInfo = scrollPoints_->at(nextI);

        resetTweens(c, tweenPoints_->at(i), currentViewInfo, nextViewInfo, 0);

        circularIncrement(spriteIndex, spriteList_);
    }
}

void ScrollingList::destroyItems()
{
    if(!spriteList_)
    {
        return;
    }
    std::vector<ComponentItemBinding *>::iterator it  = spriteList_->begin();

    while(it != spriteList_->end())
    {
        if(*it != NULL)
        {
            deallocateTexture(*it);

            if((*it)->item)
            {
                delete (*it)->item;
            }

            delete *it;
        }


        spriteList_->erase(it);

        it = spriteList_->begin();
    }

    delete spriteList_;
    spriteList_ = NULL;
}


void ScrollingList::setPoints(std::vector<ViewInfo *> *scrollPoints, std::vector<AnimationEvents *> *tweenPoints)
{
    scrollPoints_ = scrollPoints;
    tweenPoints_ = tweenPoints;
}

unsigned int ScrollingList::getScrollOffsetIndex()
{
    return firstSpriteIndex_;
}

void ScrollingList::setScrollOffsetIndex(unsigned int index)
{
    if(spriteList_ && index < spriteList_->size())
    {
        deallocateSpritePoints();
        firstSpriteIndex_ = index;
        allocateSpritePoints();
    }
}

void ScrollingList::setSelectedIndex(int selectedIndex)
{
    selectedSpriteListIndex_ = selectedIndex;

    for(unsigned int i = 0; spriteList_ && scrollPoints_ && i < selectedSpriteListIndex_; ++i)
    {
        circularDecrement(firstSpriteIndex_, spriteList_);
    }
}

void ScrollingList::click(double nextScrollTime)
{
    if(!spriteList_)
    {
        return;
    }
    if(spriteList_->size() == 0)
    {
        return;
    }

    unsigned int listSize = scrollPoints_->size();
    unsigned int end = circularIncrement(firstSpriteIndex_, listSize - 1, spriteList_);
    unsigned int allocSpriteIndex = 0;
    unsigned int deallocSpriteIndex = 0;
    unsigned int allocPoint = 0;

    if(currentScrollDirection_ == ScrollDirectionBack)
    {
        deallocSpriteIndex = end;
        circularDecrement(firstSpriteIndex_, spriteList_);
        allocSpriteIndex = firstSpriteIndex_;
        allocPoint = 0;
    }
    else if(currentScrollDirection_ == ScrollDirectionForward)
    {
        deallocSpriteIndex = firstSpriteIndex_;
        circularIncrement(firstSpriteIndex_, spriteList_);
        allocSpriteIndex = circularIncrement(firstSpriteIndex_, listSize - 1, spriteList_);
        allocPoint = listSize - 1;
    }
    else
    {
        return;
    }

    deallocateTexture(spriteList_->at(deallocSpriteIndex));
    allocateTexture(spriteList_->at(allocSpriteIndex));

    Component *c = spriteList_->at(allocSpriteIndex)->component;
    ViewInfo *currentViewInfo = scrollPoints_->at(allocPoint);
    unsigned int nextI = getNextTween(allocPoint, scrollPoints_);
    ViewInfo *nextViewInfo = scrollPoints_->at(nextI);

    resetTweens(c, tweenPoints_->at(allocPoint), currentViewInfo, nextViewInfo, nextScrollTime);
}

unsigned int ScrollingList::getNextTween(unsigned int currentIndex, std::vector<ViewInfo *> *list)
{
    if(currentScrollDirection_ == ScrollDirectionForward)
    {
        circularDecrement(currentIndex, list);
    }
    else if(currentScrollDirection_ == ScrollDirectionBack)
    {
        circularIncrement(currentIndex, list);
    }

    return currentIndex;
}

void ScrollingList::pageUp()
{
    notifyAllRequested_ = true;
    deallocateSpritePoints();

    if(spriteList_ && scrollPoints_ && scrollPoints_->size() > 2)
    {
        scrollPeriod_ = 0;
        unsigned int counts = scrollPoints_->size() - 2;

        for(unsigned int i = 0; i < counts; i++)
        {
            circularDecrement(firstSpriteIndex_, spriteList_);
        }
    }

    allocateSpritePoints();

//    CurrentScrollState = ScrollStatePageChange;
}

void ScrollingList::pageDown()
{
    notifyAllRequested_ = true;

    deallocateSpritePoints();

    if(spriteList_ && scrollPoints_ && scrollPoints_->size() > 2)
    {
        unsigned int counts = scrollPoints_->size() - 2;

        scrollPeriod_ = 0;
        for(unsigned int i = 0; i < counts; i++)
        {
            circularIncrement(firstSpriteIndex_, spriteList_);
        }
    }

    allocateSpritePoints();
}


void ScrollingList::letterUp()
{
    notifyAllRequested_ = true;
    deallocateSpritePoints();

    if(spriteList_ && scrollPoints_ && getSelectedCollectionItemSprite())
    {
        unsigned int i = 0;

        // Select the previous item in the list in case we are at the top of all the items
        // for the currently selected letter. 
        circularDecrement(firstSpriteIndex_, spriteList_);
        std::string startname = getSelectedCollectionItemSprite()->item->lowercaseFullTitle();
        ++i;

        bool done = false;

        // traverse up through the list until we find the first item that starts with a different letter
        while(!done && i < spriteList_->size())
        {
            circularDecrement(firstSpriteIndex_, spriteList_);
            std::string endname = getSelectedCollectionItemSprite()->item->lowercaseFullTitle();
            ++i;
            
            // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
            if(isalpha(startname[0]) ^ isalpha(endname[0]))
            {
                done = true;
            }
            else if(isalpha(startname[0]) && isalpha(endname[0]) && startname[0] != endname[0])
            {
                done = true;
            }
            
            if(done)
            {
                // our searching went too far, rewind to the first item in the list that matches the starting letter
                circularIncrement(firstSpriteIndex_, spriteList_);
            }
        }
    }

    allocateSpritePoints();
}

void ScrollingList::letterDown()
{
    notifyAllRequested_ = true;
    deallocateSpritePoints();

    if(spriteList_ && scrollPoints_ && getSelectedCollectionItemSprite())
    {

        std::string startname = getSelectedCollectionItemSprite()->item->lowercaseFullTitle();
        std::string endname = startname;

        unsigned int i = 0;
        bool done = false;
        while(!done && i < spriteList_->size())
        {
            circularIncrement(firstSpriteIndex_, spriteList_);
            endname = getSelectedCollectionItemSprite()->item->lowercaseFullTitle();
            ++i;
            
            // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
            if(isalpha(startname[0]) ^ isalpha(endname[0]))
            {
                done = true;
            }
            else if(isalpha(startname[0]) && isalpha(endname[0]) && startname[0] != endname[0])
            {
                done = true;
            }
        }
    }

    allocateSpritePoints();
}


void ScrollingList::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
    tweenEnterTime_ = 0;
    currentScrollDirection_ = ScrollDirectionIdle;
    requestedScrollDirection_ = ScrollDirectionIdle;
    currentScrollState_ = ScrollStateIdle;
    scrollPeriod_ = 0;

    for(unsigned int i = 0; spriteList_ && i < spriteList_->size(); i++)
    {
        ComponentItemBinding *s = spriteList_->at(i);

        deallocateTexture(s);
    }
}

void ScrollingList::triggerMenuEnterEvent()
{
    focus_ = true;
    notifyAllRequested_ = true;

    if(!scrollPoints_)
    {
        return;
    }
    if(!spriteList_)
    {
        return;
    }
    if(spriteList_->size() == 0 )
    {
        return;
    }
    if(firstSpriteIndex_ >= spriteList_->size())
    {
        return;
    }

    unsigned int spriteIndex = firstSpriteIndex_;

    for(unsigned int i = 0; i < scrollPoints_->size(); ++i)
    {
        ComponentItemBinding *s = spriteList_->at(spriteIndex);

        Component *c = s->component;
        if(c)
        {
            c->triggerMenuEnterEvent();
        }

        circularIncrement(spriteIndex, spriteList_);
    }
}

void ScrollingList::triggerMenuExitEvent()
{
    focus_ = false;
    notifyAllRequested_ = true;

    if(!scrollPoints_)
    {
        return;
    }
    if(!spriteList_)
    {
        return;
    }
    if(spriteList_->size() == 0 )
    {
        return;
    }
    if(firstSpriteIndex_ >= spriteList_->size())
    {
        return;
    }

    unsigned int spriteIndex = firstSpriteIndex_;

    for(unsigned int i = 0; i < scrollPoints_->size(); ++i)
    {
        ComponentItemBinding *s = spriteList_->at(spriteIndex);

        Component *c = s->component;
        if(c)
        {
            c->triggerMenuExitEvent();
        }

        circularIncrement(spriteIndex, spriteList_);
    }
}

void ScrollingList::update(float dt)
{
    Component::update(dt);

    if(!scrollPoints_)
    {
        return;
    }
    if(!spriteList_)
    {
        return;
    }
    if(spriteList_->size() == 0)
    {
        return;
    }

    bool readyToScroll = true;
    bool scrollChanged = false;
    bool scrollRequested = false;
    bool scrollStopped = false;

    // validate all scroll points are done tweening to the next position
    for(unsigned int i = 0; i < spriteList_->size(); i++)
    {
        ComponentItemBinding *s = spriteList_->at(i);
        Component *c = s->component;

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
            click(0);
            currentScrollState_ = ScrollStateIdle;
            scrollStopped = true;

            // update the tweens now that we are done
            unsigned int spriteIndex = firstSpriteIndex_;

            for(unsigned int i = 0; i < tweenPoints_->size(); ++i)
            {
                ComponentItemBinding *s = spriteList_->at(spriteIndex);

                Component *c = s->component;
                if(c)
                {
                    c->setTweens(tweenPoints_->at(i));
                }

                circularIncrement(spriteIndex, spriteList_);
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


    unsigned int spriteIndex = firstSpriteIndex_;
    for(unsigned int i = 0; i < scrollPoints_->size(); i++)
    {
        updateSprite(spriteIndex, i, (scrollRequested || scrollChanged), dt, scrollPeriod_);
        circularIncrement(spriteIndex, spriteList_);
    }

    if(scrollStopped || (notifyAllRequested_ && focus_))
    {
        ComponentItemBinding *sprite = getPendingCollectionItemSprite();
        Item *item = NULL;

        if(sprite)
        {
            item = sprite->item;
        }

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

void ScrollingList::updateSprite(unsigned int spriteIndex, unsigned int pointIndex, bool newScroll, float dt, double nextScrollTime)
{
    ComponentItemBinding *s = spriteList_->at(spriteIndex);

    Component *c = s->component;
    //todo: remove me
    if(c && newScroll)
    {
        ViewInfo *currentViewInfo = scrollPoints_->at(pointIndex);
        unsigned int nextI = getNextTween(pointIndex, scrollPoints_);
        ViewInfo *nextViewInfo = scrollPoints_->at(nextI);

        resetTweens(c, tweenPoints_->at(pointIndex), currentViewInfo, nextViewInfo, nextScrollTime);
        c->triggerMenuScrollEvent();
    }
    if(c)
    {
        c->update(dt);
    }

    circularIncrement(spriteIndex, spriteList_);
}

void ScrollingList::resetTweens(Component *c, AnimationEvents *sets, ViewInfo *currentViewInfo, ViewInfo *nextViewInfo, double scrollTime)
{
    if(!c)
    {
        return;
    }
    if(!sets)
    {
        return;
    }
    if(!currentViewInfo)
    {
        return;
    }
    if(!nextViewInfo)
    {
        return;
    }

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


bool ScrollingList::allocateTexture(ComponentItemBinding *s)
{

    if(!s || s->component != NULL)
    {
        return false;
    }

    const Item *item = s->item;
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
        s->component = t;
    }

    return true;
}

void ScrollingList::deallocateTexture(ComponentItemBinding *s)
{
    if(s && s->component != NULL)
    {
        delete s->component;
        //todo: memory leak here, need to destroy allocated tween points here and in page (cannot be destroyed by component)
    }
    s->component = NULL;
}

void ScrollingList::draw()
{
    //todo: Poor design implementation.
    // caller should instead call ScrollingList::Draw(unsigned int layer)
}

void ScrollingList::draw(unsigned int layer)
{
    if(!scrollPoints_)
    {
        return;
    }
    if(!spriteList_)
    {
        return;
    }
    if(spriteList_->size() == 0)
    {
        return;
    }

    unsigned int spriteIndex = firstSpriteIndex_;

    for(unsigned int i = 0; i < scrollPoints_->size(); i++)
    {
        ComponentItemBinding *s = spriteList_->at(spriteIndex);
        Component *c = s->component;
        ViewInfo *currentViewInfo = scrollPoints_->at(i);

        if(c && currentViewInfo && currentViewInfo->Layer == layer)
        {
            c->draw();
        }
        circularIncrement(spriteIndex, spriteList_);
    }
}


void ScrollingList::setScrollDirection(ScrollDirection direction)
{
    requestedScrollDirection_ = direction;

    scrollStopRequested_ = (direction == ScrollDirectionIdle);
}

void ScrollingList::removeSelectedItem()
{
    ComponentItemBinding *sprite = getSelectedCollectionItemSprite();
    if(sprite && spriteList_)
    {
        Item *item = sprite->item;
        deallocateTexture(sprite);
        int index = (firstSpriteIndex_ + selectedSpriteListIndex_) % spriteList_->size();

        std::vector<ComponentItemBinding *>::iterator it = spriteList_->begin() + index;

        spriteList_->erase(it);
        delete sprite;

        if(item)
        {
            delete item;
        }

        if(selectedSpriteListIndex_ >= spriteList_->size())
        {
            selectedSpriteListIndex_ = 0;
        }
        if(firstSpriteIndex_ >= spriteList_->size())
        {
            firstSpriteIndex_ = 0;
        }
    }
}


std::vector<ComponentItemBinding *> *ScrollingList::getCollectionItemSprites()
{
    return spriteList_;
}

ComponentItemBinding* ScrollingList::getSelectedCollectionItemSprite()
{
    ComponentItemBinding *item = NULL;

    if(spriteList_ && spriteList_->size() > 0)
    {
        int index = (firstSpriteIndex_ + selectedSpriteListIndex_) % spriteList_->size();

        item = spriteList_->at(index);
    }

    return item;
}

ComponentItemBinding* ScrollingList::getPendingCollectionItemSprite()
{
    ComponentItemBinding *item = NULL;
    unsigned int index = firstSpriteIndex_;
    if(spriteList_ && spriteList_->size() > 0)
    {
        index = (index + selectedSpriteListIndex_) % spriteList_->size();

        item = spriteList_->at(index);
    }

    return item;
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


ComponentItemBinding* ScrollingList::getPendingSelectedCollectionItemSprite()
{
    ComponentItemBinding *item = NULL;

    if(spriteList_)
    {
        unsigned int index = selectedSpriteListIndex_;

        if(currentScrollDirection_ == ScrollDirectionBack)
        {
            circularDecrement(index, spriteList_);
        }
        if(currentScrollDirection_ == ScrollDirectionForward)
        {
            circularIncrement(index, spriteList_);
        }

        if(spriteList_ && spriteList_->size() > 0)
        {
            index = (index + selectedSpriteListIndex_) % spriteList_->size();

            item = spriteList_->at(index);
        }
    }

    return item;
}

bool ScrollingList::isIdle()
{
    return (Component::isIdle() && currentScrollState_ == ScrollStateIdle);
}

void ScrollingList::circularIncrement(unsigned int &index, std::vector<ViewInfo*>* list)
{
    index++;

    if(index >= list->size())
    {
        index = 0;
    }
}

void ScrollingList::circularDecrement(unsigned int &index, std::vector<ViewInfo*>* list)
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

int ScrollingList::circularIncrement(unsigned int index, unsigned int offset, std::vector<ComponentItemBinding *> *list)
{
    unsigned int end = index + offset;

    while(end >= list->size() && list->size() > 0)
    {
        end -= list->size();
    }

    return end;
}


void ScrollingList::circularIncrement(unsigned int &index, std::vector<ComponentItemBinding*> *list)
{
    index++;

    if(index >= list->size())
    {
        index = 0;
    }
}

void ScrollingList::circularDecrement(unsigned int &index, std::vector<ComponentItemBinding*> *list)
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

