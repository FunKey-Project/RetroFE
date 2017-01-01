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

#include "ScrollingList.h"
#include "../Animate/Tween.h"
#include "../Animate/TweenSet.h"
#include "../Animate/Animation.h"
#include "../Animate/AnimationEvents.h"
#include "../Animate/TweenTypes.h"
#include "../Font.h"
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
#include <iomanip>

//todo: remove coupling from configuration data (if possible)
ScrollingList::ScrollingList(Configuration &c,
                             Page &p,
                             bool layoutMode,
                             float scaleX,
                             float scaleY,
                             Font *font,
                             std::string layoutKey,
                             std::string imageType)
    : Component(p)
    , horizontalScroll(false)
    , layoutMode_(layoutMode)
    , spriteList_(NULL)
    , scrollPoints_(NULL)
    , tweenPoints_(NULL)
    , itemIndex_(0)
    , componentIndex_(0)
    , selectedOffsetIndex_(0)
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
    , itemIndex_(0)
    , componentIndex_(0)
    , selectedOffsetIndex_(copy.selectedOffsetIndex_)
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

    setPoints(copy.scrollPoints_, copy.tweenPoints_);

}


ScrollingList::~ScrollingList()
{
    destroyItems();
}


void ScrollingList::setItems(std::vector<Item *> *items)
{
    items_ = items;
    if(items_)
    {
        itemIndex_ = loopDecrement(0, selectedOffsetIndex_, items_->size());
    }
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
    if(!items_ || items_->size() == 0) return;
    if(!scrollPoints_) return;
    if(components_.size() == 0) return;

    for(unsigned int i = 0; i < scrollPoints_->size(); ++i)
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
    scrollPoints_ = scrollPoints;
    tweenPoints_ = tweenPoints;

    // empty out the list as we will resize it
    components_.clear();

    int size = 0;

    if(scrollPoints) size = scrollPoints_->size();
    components_.resize(size);

    if(items_)
    {
        itemIndex_ = loopDecrement(0, selectedOffsetIndex_, items_->size());
    }
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

Item *ScrollingList::getItemByOffset(int offset)
{
    if(!items_ || items_->size() == 0) return NULL;
    unsigned int index = getSelectedIndex();
    if(offset > 0) {
        index = loopIncrement(index, offset, items_->size());
    }
    else if(offset < 0) {
        index = loopDecrement(index, offset*-1, items_->size());
    }
    
    return items_->at(index);
}


Item *ScrollingList::getSelectedItem()
{
    if(!items_ || items_->size() == 0) return NULL;
    unsigned index = loopIncrement(itemIndex_, selectedOffsetIndex_, items_->size());
    return items_->at(index);
}


void ScrollingList::pageUp()
{
    if(components_.size() == 0) return;
    itemIndex_ = loopDecrement(itemIndex_, components_.size(), items_->size());
}

void ScrollingList::pageDown()
{
    if(components_.size() == 0) return;
    itemIndex_ = loopIncrement(itemIndex_, components_.size(), items_->size());
}

void ScrollingList::random()
{
    if(!items_ || items_->size() == 0) return;
    itemIndex_ = rand() % items_->size();
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
    std::string startname = items_->at((itemIndex_+selectedOffsetIndex_)%items_->size())->lowercaseFullTitle();

    for(unsigned int i = 0; i < items_->size(); ++i)
    {
        unsigned int index = 0;
        if(increment) index = loopIncrement(itemIndex_, i, items_->size());
        else index = loopDecrement(itemIndex_, i, items_->size());

        std::string endname = items_->at((index+selectedOffsetIndex_)%items_->size())->lowercaseFullTitle();

        // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
        if((isalpha(startname[0]) ^ isalpha(endname[0])) ||
           (isalpha(startname[0]) && isalpha(endname[0]) && startname[0] != endname[0]))
        {
            itemIndex_ = index;
            break;
        }
    }

    if (!increment) // For decrement, find the first game of the new letter
    {
        startname = items_->at((itemIndex_+selectedOffsetIndex_)%items_->size())->lowercaseFullTitle();

        for(unsigned int i = 0; i < items_->size(); ++i)
        {
            unsigned int index = loopDecrement(itemIndex_, i, items_->size());

            std::string endname = items_->at((index+selectedOffsetIndex_)%items_->size())->lowercaseFullTitle();

            // check if we are changing characters from a-z, or changing from alpha character to non-alpha character
            if((isalpha(startname[0]) ^ isalpha(endname[0])) ||
               (isalpha(startname[0]) && isalpha(endname[0]) && startname[0] != endname[0]))
            {
                itemIndex_ = loopIncrement(index,1,items_->size());
                break;
            }
        }
    }

}


void ScrollingList::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
    scrollPeriod_ = 0;
    
    deallocateSpritePoints();
}

void ScrollingList::triggerEnterEvent()
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerEvent( "enter" );
    }
}

void ScrollingList::triggerExitEvent()
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerEvent( "exit" );
    }
}

void ScrollingList::triggerMenuEnterEvent( int menuIndex )
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerEvent( "menuEnter", menuIndex );
    }
}

void ScrollingList::triggerMenuExitEvent( int menuIndex )
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerEvent( "menuExit", menuIndex );
    }
}

void ScrollingList::triggerHighlightEnterEvent( int menuIndex )
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerEvent( "highlightEnter", menuIndex );
    }
}

void ScrollingList::triggerHighlightExitEvent( int menuIndex )
{
    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c) c->triggerEvent( "highlightExit", menuIndex );
    }
}

void ScrollingList::update(float dt)
{
    Component::update(dt);

    if(components_.size() == 0) return;
    if(!items_ || items_->size() == 0) return;

    for(unsigned int i = 0; i < scrollPoints_->size(); i++)
    {
        unsigned int cindex = loopIncrement(componentIndex_, i, components_.size());
        Component *c = components_.at(cindex);
        if(c) c->update(dt);
    }
}

unsigned int ScrollingList::getSelectedIndex()
{
     if(!items_) return 0;
    return loopIncrement(itemIndex_, selectedOffsetIndex_, items_->size());
}

unsigned int ScrollingList::getSize()
{
    if(!items_) return 0;

    return items_->size();
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
    set->push(new Tween(TWEEN_PROPERTY_HEIGHT, EASE_INOUT_QUADRATIC, currentViewInfo->Height, nextViewInfo->Height, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_WIDTH, EASE_INOUT_QUADRATIC, currentViewInfo->Width, nextViewInfo->Width, scrollTime));
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
    set->push(new Tween(TWEEN_PROPERTY_MAX_WIDTH, EASE_INOUT_QUADRATIC, currentViewInfo->MaxWidth, nextViewInfo->MaxWidth, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_MAX_HEIGHT, EASE_INOUT_QUADRATIC, currentViewInfo->MaxHeight, nextViewInfo->MaxHeight, scrollTime));
    set->push(new Tween(TWEEN_PROPERTY_LAYER, EASE_INOUT_QUADRATIC, currentViewInfo->Layer, nextViewInfo->Layer, scrollTime));
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

    std::string layoutName;
    config_.getProperty("layout", layoutName);

    // check collection path for art based on gamename
    if (layoutMode_)
    {
        imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", collectionName);
        imagePath = Utils::combinePath(imagePath, "medium_artwork", imageType_);
    }
    else
    {
        config_.getMediaPropertyAbsolutePath(collectionName, imageType_, false, imagePath);
    }
    t = imageBuild.CreateImage(imagePath, page, item->name, scaleX_, scaleY_);

    // check sub-collection path for art based on gamename
    if(!t)
    {
        if (layoutMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", item->collectionInfo->name);
            imagePath = Utils::combinePath(imagePath, "medium_artwork", imageType_);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(item->collectionInfo->name, imageType_, false, imagePath);
        }
        t = imageBuild.CreateImage(imagePath, page, item->name, scaleX_, scaleY_);
    }

    // check collection path for art based on game name (full title)
    if(!t && item->title != item->fullTitle)
    {
        if (layoutMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", collectionName);
            imagePath = Utils::combinePath(imagePath, "medium_artwork", imageType_);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(collectionName, imageType_, false, imagePath);
        }
        t = imageBuild.CreateImage(imagePath, page, item->fullTitle, scaleX_, scaleY_);
    }

    // check sub-collection path for art based on game name (full title)
    if(!t && item->title != item->fullTitle)
    {
        if (layoutMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", item->collectionInfo->name);
            imagePath = Utils::combinePath(imagePath, "medium_artwork", imageType_);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(item->collectionInfo->name, imageType_, false, imagePath);
        }
        t = imageBuild.CreateImage(imagePath, page, item->fullTitle, scaleX_, scaleY_);
    }

    // check collection path for art based on parent game name
    if(!t && item->cloneof != "")
    {
        if (layoutMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", collectionName);
            imagePath = Utils::combinePath(imagePath, "medium_artwork", imageType_);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(collectionName, imageType_, false, imagePath);
        }
        t = imageBuild.CreateImage(imagePath, page, item->cloneof, scaleX_, scaleY_);
    }

    // check sub-collection path for art based on parent game name
    if(!t && item->cloneof != "")
    {
        if (layoutMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", item->collectionInfo->name);
            imagePath = Utils::combinePath(imagePath, "medium_artwork", imageType_);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(item->collectionInfo->name, imageType_, false, imagePath);
        }
        t = imageBuild.CreateImage(imagePath, page, item->cloneof, scaleX_, scaleY_);
    }

    // check collection path for art based on system name
    if(!t)
    {
        if (layoutMode_)
        {
            imagePath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", item->name);
            imagePath = Utils::combinePath(imagePath, "system_artwork");
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(item->name, imageType_, true, imagePath);
        }
        t = imageBuild.CreateImage(imagePath, page, imageType_, scaleX_, scaleY_);
    }

    if (!t)
    {
        t = new Text(item->title, page, fontInst_, scaleX_, scaleY_);
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
    
    if(components_.size() == 0) return;

    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c && c->baseViewInfo.Layer == layer) c->draw();
    }
}


bool ScrollingList::isIdle()
{
    if(!Component::isIdle()) return false;

    for(unsigned int i = 0; i < components_.size(); ++i)
    {
        Component *c = components_.at(i);
        if(c && !c->isIdle()) return false;
    }

    return true;
}


void ScrollingList::resetScrollPeriod()
{
    scrollPeriod_ = startScrollTime_;
    return;
}


void ScrollingList::updateScrollPeriod()
{
    scrollPeriod_ -= scrollAcceleration_;
    if(scrollPeriod_ < scrollAcceleration_)
    {
        scrollPeriod_ = scrollAcceleration_;
    }
}


void ScrollingList::scroll(bool forward)
{

    if(forward)
    {
        Item *i = items_->at(loopIncrement(itemIndex_, scrollPoints_->size(), items_->size()));
        deallocateTexture(componentIndex_);
        allocateTexture(componentIndex_, i);
    }
    else
    {
        Item *i = items_->at(loopDecrement(itemIndex_, 1, items_->size()));
        deallocateTexture(loopDecrement(componentIndex_, 1, components_.size()));
        allocateTexture(loopDecrement(componentIndex_, 1, components_.size()), i);
    }

    for(unsigned int i = 0; i < scrollPoints_->size(); i++)
    {
        unsigned int cindex = loopIncrement(componentIndex_, i, components_.size());

        Component *c = components_.at(cindex);

        unsigned int nextI = 0;
        if(forward)
        {
            nextI = loopDecrement(i, 1, scrollPoints_->size());
        }
        else
        {
            nextI = loopIncrement(i, 1, scrollPoints_->size());
        }

        ViewInfo *currentvi = scrollPoints_->at(i);
        ViewInfo *nextvi = scrollPoints_->at(nextI);

        resetTweens(c, tweenPoints_->at(i), currentvi, nextvi, scrollPeriod_);
        c->baseViewInfo.font = nextvi->font; // Use the font settings of the next index
        c->triggerEvent( "menuScroll" );
    }

    if(forward)
    {
        itemIndex_ = loopIncrement(itemIndex_, 1, items_->size());
        componentIndex_ = loopIncrement(componentIndex_, 1, components_.size()); 
    }
    else
    {
        itemIndex_ = loopDecrement(itemIndex_, 1, items_->size());
        componentIndex_ = loopDecrement(componentIndex_, 1, components_.size()); 
    }

    return;
}
