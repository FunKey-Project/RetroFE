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
    tweens_ = NULL;
    newItemSelectedSinceEnter = false;
    backgroundTexture_ = NULL;
    freeGraphicsMemory();

}

Component::Component(const Component &copy)
    : page(copy.page)
{
    tweens_ = NULL;
    newItemSelectedSinceEnter = false;
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
    currentAnimationState = HIDDEN;
    enterRequested = false;
    exitRequested = false;
    menuEnterRequested = false;
    menuEnterIndex = -1;
    menuScrollRequested = false;
    menuExitRequested = false;
    menuExitIndex = -1;

    newItemSelected = false;
    playlistChanged = false;
    highlightExitComplete = false;
    currentTweens_ = NULL;
    currentTweenIndex_ = 0;
    currentTweenComplete_ = false;
    elapsedTweenTime_ = 0;
    scrollActive = false;

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

void Component::triggerEnterEvent()
{
    enterRequested = true;
}

void Component::triggerExitEvent()
{
    exitRequested = true;
}



void Component::triggerMenuEnterEvent(int menuIndex)
{
    menuEnterRequested = true;
    menuEnterIndex = menuIndex;
}

void Component::triggerMenuScrollEvent()
{
    menuScrollRequested = true;
}


void Component::triggerMenuExitEvent(int menuIndex)
{
    menuExitRequested = true;
    menuExitIndex = menuIndex;
}
void Component::triggerHighlightEvent()
{
    newItemSelected = true;
}

void Component::triggerPlaylistChangeEvent(std::string name)
{
    playlistChanged = true;
    this->playlistName = name;
}

bool Component::isIdle()
{
    return (currentAnimationState == IDLE);
}

bool Component::isHidden()
{
    return (currentAnimationState == HIDDEN);
}
bool Component::isWaiting()
{
    return (currentAnimationState == HIGHLIGHT_WAIT);
}

bool Component::isMenuScrolling()
{
    return (currentAnimationState == MENU_ENTER || currentAnimationState == MENU_SCROLL || currentAnimationState == MENU_EXIT || menuScrollRequested);
}

void Component::setTweens(AnimationEvents *set)
{
    tweens_ = set;
    forceIdle();
}

void Component::forceIdle()
{
    currentAnimationState = IDLE;
    currentTweenIndex_ = 0;
    currentTweenComplete_ = false;
    elapsedTweenTime_ = 0;
    currentTweens_ = NULL;
}


void Component::update(float dt)
{
    elapsedTweenTime_ += dt;
    highlightExitComplete = false;
    if(isHidden() || isWaiting() || (isIdle() && exitRequested))
    {
        currentTweenComplete_ = true;
    }

    if(currentTweenComplete_)
    {
        currentTweens_ = NULL;

        // There was no request to override our state path. Continue on as normal.
        std::stringstream ss;
        switch(currentAnimationState)
        {
        case MENU_ENTER:
            currentTweens_ = NULL;
            currentAnimationState = IDLE;
            break;

        case MENU_SCROLL:
            currentTweens_ = NULL;
            currentAnimationState = IDLE;
            break;

        case MENU_EXIT:
            currentTweens_ = NULL;
            currentAnimationState = IDLE;
            break;


        case ENTER:
            currentTweens_ = tweens_->getAnimation("enter", menuEnterIndex);
            currentAnimationState = HIGHLIGHT_ENTER;
            break;

        case EXIT:
            currentTweens_ = NULL;
            currentAnimationState = HIDDEN;
            break;

        case HIGHLIGHT_ENTER:
            currentTweens_ = tweens_->getAnimation("idle", menuEnterIndex);
            currentAnimationState = IDLE;
            break;

        case IDLE:
            // prevent us from automatically jumping to the exit tween upon enter
            if(enterRequested)
            {
                enterRequested = false;
                newItemSelected = false;
            }
            else if(menuExitRequested && (!menuEnterRequested || menuExitRequested <= menuEnterRequested))
            {
                currentTweens_ = tweens_->getAnimation("menuExit", menuExitIndex);
                currentAnimationState = MENU_EXIT;
                menuExitRequested = false;
            }
            else if(menuEnterRequested && (!menuExitRequested || menuExitRequested > menuEnterRequested))
            {
                currentTweens_ = tweens_->getAnimation("menuEnter", menuEnterIndex);
                currentAnimationState = MENU_ENTER;
                menuEnterRequested = false;

            }
            else if(menuScrollRequested)
            {
                menuScrollRequested = false;
                currentTweens_ = tweens_->getAnimation("menuScroll", menuEnterIndex);
                currentAnimationState = MENU_SCROLL;
            }
            else if(scrollActive || newItemSelected || exitRequested)
            {
                currentTweens_ = tweens_->getAnimation("highlightExit", menuEnterIndex);
                currentAnimationState = HIGHLIGHT_EXIT;
            }
            else
            {
                currentTweens_ = tweens_->getAnimation("idle", menuEnterIndex);
                currentAnimationState = IDLE;
            }
            break;

        case HIGHLIGHT_EXIT:

            // intentionally break down
        case HIGHLIGHT_WAIT:

            if(exitRequested && (currentAnimationState == HIGHLIGHT_WAIT))
            {
                currentTweens_ = tweens_->getAnimation("highlightExit", menuEnterIndex);
                currentAnimationState = HIGHLIGHT_EXIT;

            }
            else if(exitRequested && (currentAnimationState == HIGHLIGHT_EXIT))
            {

                currentTweens_ = tweens_->getAnimation("exit", menuEnterIndex);
                currentAnimationState = EXIT;
                exitRequested = false;
            }
            else if(scrollActive)
            {
                currentTweens_ = NULL;
                currentAnimationState = HIGHLIGHT_WAIT;
            }
            else if(newItemSelected)
            {
                currentTweens_ = tweens_->getAnimation("highlightEnter", menuEnterIndex);
                currentAnimationState = HIGHLIGHT_ENTER;
                highlightExitComplete = true;
                newItemSelected = false;
            }
            else
            {
                currentTweens_ = NULL;
                currentAnimationState = HIGHLIGHT_WAIT;
            }
            break;

        case HIDDEN:
            if(enterRequested || exitRequested)
            {
                currentTweens_ = tweens_->getAnimation("enter", menuEnterIndex);
                currentAnimationState = ENTER;
            }

            else if(menuExitRequested && (!menuEnterRequested || menuExitRequested <= menuEnterRequested))
            {
                currentTweens_ = tweens_->getAnimation("menuExit", menuExitIndex);
                currentAnimationState = MENU_EXIT;
                menuExitRequested = false;
            }
            else if(menuEnterRequested && (!menuExitRequested || menuExitRequested > menuEnterRequested))
            {
                currentTweens_ = tweens_->getAnimation("menuEnter", menuEnterIndex);
                currentAnimationState = MENU_ENTER;
                menuEnterRequested = false;

            }
            else if(menuScrollRequested)
            {
                menuScrollRequested = false;
                currentTweens_ = tweens_->getAnimation("menuScroll", menuEnterIndex);
                currentAnimationState = MENU_SCROLL;
            }
            else
            {
                currentTweens_ = NULL;
                currentAnimationState = HIDDEN;
            }
        }

        currentTweenIndex_ = 0;
        currentTweenComplete_ = false;

        elapsedTweenTime_ = 0;
    }

    currentTweenComplete_ = animate(isIdle());
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

bool Component::animate(bool loop)
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
        if(loop)
        {
            currentTweenIndex_ = 0;
        }
        completeDone = true;
    }

    return completeDone;
}
