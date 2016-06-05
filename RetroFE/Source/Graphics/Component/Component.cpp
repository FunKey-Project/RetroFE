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
#include "Component.h"
#include "../Animate/Tween.h"
#include "../../Graphics/ViewInfo.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"

Component::Component(Page &p)
: page(p)
{
    tweens_                   = NULL;
    backgroundTexture_        = NULL;
    freeGraphicsMemory();

}

Component::Component(const Component &copy)
    : page(copy.page)
{
    tweens_ = NULL;
    backgroundTexture_ = NULL;
    freeGraphicsMemory();

    if(copy.tweens_)
    {
        AnimationEvents *tweens = new AnimationEvents(*copy.tweens_);
        setTweens(tweens);
    }


}

Component::~Component()
{
    freeGraphicsMemory();
}

void Component::freeGraphicsMemory()
{
    animationRequestedType_ = "";
    animationType_          = "";
    animationRequested_     = false;
    newItemSelected         = false;
    menuIndex_              = -1;

    currentTweens_        = NULL;
    currentTweenIndex_    = 0;
    currentTweenComplete_ = true;
    elapsedTweenTime_     = 0;

    if(backgroundTexture_)
    {
        SDL_LockMutex(SDL::getMutex());
        SDL_DestroyTexture(backgroundTexture_);
        SDL_UnlockMutex(SDL::getMutex());

        backgroundTexture_ = NULL;
    }
}
void Component::allocateGraphicsMemory()
{
    if(!backgroundTexture_)
    {
        // make a 4x4 pixel wide surface to be stretched during rendering, make it a white background so we can use
        // color  later
        SDL_Surface *surface = SDL_CreateRGBSurface(0, 4, 4, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

        SDL_LockMutex(SDL::getMutex());
        backgroundTexture_ = SDL_CreateTextureFromSurface(SDL::getRenderer(), surface);
        SDL_UnlockMutex(SDL::getMutex());

        SDL_FreeSurface(surface);
        SDL_SetTextureBlendMode(backgroundTexture_, SDL_BLENDMODE_BLEND);
    }
}

void Component::triggerEvent(std::string event, int menuIndex)
{
    animationRequestedType_ = event;
    animationRequested_     = true;
    menuIndex_              = menuIndex;
}

void Component::setPlaylist(std::string name)
{
    this->playlistName = name;
}

void Component::setNewItemSelected()
{
  newItemSelected = true;
}

bool Component::isIdle()
{
    return (currentTweenComplete_ || animationType_ == "idle");
}

bool Component::isMenuScrolling()
{
    return (!currentTweenComplete_ && animationType_ == "menuScroll");
}

void Component::setTweens(AnimationEvents *set)
{
    tweens_ = set;
}

void Component::update(float dt)
{
    elapsedTweenTime_ += dt;

    if(animationRequested_ && animationRequestedType_ != "")
    {
      Animation *newTweens  = tweens_->getAnimation( animationRequestedType_, menuIndex_ );
      if (newTweens)
      {
        animationType_        = animationRequestedType_;
        currentTweens_        = newTweens;
        currentTweenIndex_    = 0;
        elapsedTweenTime_     = 0;
        currentTweenComplete_ = false;
      }
      animationRequested_   = false;
    }
    else if (tweens_ && currentTweenComplete_)
    {
      animationType_          = "idle";
      currentTweens_          = tweens_->getAnimation( "idle", menuIndex_ );
      currentTweenIndex_      = 0;
      elapsedTweenTime_       = 0;
      currentTweenComplete_   = false;
      animationRequested_     = false;
    }

    currentTweenComplete_ = animate();
    if(currentTweenComplete_)
    {
      currentTweens_     = NULL;
      currentTweenIndex_ = 0;
    }
}

void Component::draw()
{

    if(backgroundTexture_)
    {
        SDL_Rect rect;
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());
        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());


        SDL_SetTextureColorMod(backgroundTexture_,
                               static_cast<char>(baseViewInfo.BackgroundRed*255),
                               static_cast<char>(baseViewInfo.BackgroundGreen*255),
                               static_cast<char>(baseViewInfo.BackgroundBlue*255));

        SDL::renderCopy(backgroundTexture_, static_cast<char>(baseViewInfo.BackgroundAlpha*255), NULL, &rect, baseViewInfo.Angle);
    }
}

bool Component::animate()
{
    bool completeDone = false;
    if(!currentTweens_ || currentTweenIndex_ >= currentTweens_->size())
    {
        completeDone = true;
    }
    else if(currentTweens_)
    {
        bool currentDone = true;
        TweenSet *tweens = currentTweens_->tweenSet(currentTweenIndex_);

        for(unsigned int i = 0; i < tweens->size(); i++)
        {
            Tween *tween = tweens->tweens()->at(i);
            double elapsedTime = elapsedTweenTime_;

            //todo: too many levels of nesting
            if(elapsedTime < tween->duration)
            {
                currentDone = false;
            }
            else
            {
                elapsedTime = static_cast<float>(tween->duration);
            }

            float value = tween->animate(elapsedTime);

            switch(tween->property)
            {
            case TWEEN_PROPERTY_X:
                baseViewInfo.X = value;
                break;

            case TWEEN_PROPERTY_Y:
                baseViewInfo.Y = value;
                break;

            case TWEEN_PROPERTY_HEIGHT:
                baseViewInfo.Height = value;
                break;

            case TWEEN_PROPERTY_WIDTH:
                baseViewInfo.Width = value;
                break;

            case TWEEN_PROPERTY_ANGLE:
                baseViewInfo.Angle = value;
                break;

            case TWEEN_PROPERTY_ALPHA:
                baseViewInfo.Alpha = value;
                break;

            case TWEEN_PROPERTY_X_ORIGIN:
                baseViewInfo.XOrigin = value;
                break;

            case TWEEN_PROPERTY_Y_ORIGIN:
                baseViewInfo.YOrigin = value;
                break;

            case TWEEN_PROPERTY_X_OFFSET:
                baseViewInfo.XOffset = value;
                break;

            case TWEEN_PROPERTY_Y_OFFSET:
                baseViewInfo.YOffset = value;
                break;

            case TWEEN_PROPERTY_FONT_SIZE:
                baseViewInfo.FontSize = value;
                break;

            case TWEEN_PROPERTY_BACKGROUND_ALPHA:
                baseViewInfo.BackgroundAlpha = value;
                break;
            }
        }

        if(currentDone)
        {
            currentTweenIndex_++;
            elapsedTweenTime_ = 0;
        }
    }

    if(!currentTweens_ || currentTweenIndex_ >= currentTweens_->tweenSets()->size())
    {
        completeDone = true;
    }

    return completeDone;
}
