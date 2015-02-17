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

Component::Component()
{
    Tweens = NULL;
    SelectedItem = NULL;
    NewItemSelectedSinceEnter = false;
    BackgroundTexture = NULL;
    FreeGraphicsMemory();

}

Component::~Component()
{
    FreeGraphicsMemory();
}

void Component::FreeGraphicsMemory()
{
    CurrentAnimationState = HIDDEN;
    EnterRequested = false;
    ExitRequested = false;
    MenuEnterRequested = false;
    MenuEnterIndex = -1;
    MenuScrollRequested = false;
    MenuExitRequested = false;
    MenuExitIndex = -1;

    NewItemSelected = false;
    HighlightExitComplete = false;
    CurrentTweens = NULL;
    CurrentTweenIndex = 0;
    CurrentTweenComplete = false;
    ElapsedTweenTime = 0;
    ScrollActive = false;

    if(BackgroundTexture)
    {
        SDL_LockMutex(SDL::GetMutex());
        SDL_DestroyTexture(BackgroundTexture);
        SDL_UnlockMutex(SDL::GetMutex());

        BackgroundTexture = NULL;
    }
}
void Component::AllocateGraphicsMemory()
{
    if(!BackgroundTexture)
    {
        // make a 4x4 pixel wide surface to be stretched during rendering, make it a white background so we can use
        // color  later
        SDL_Surface *surface = SDL_CreateRGBSurface(0, 4, 4, 32, 0, 0, 0, 0);
        SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));

        SDL_LockMutex(SDL::GetMutex());
        BackgroundTexture = SDL_CreateTextureFromSurface(SDL::GetRenderer(), surface);
        SDL_UnlockMutex(SDL::GetMutex());

        SDL_FreeSurface(surface);
        SDL_SetTextureBlendMode(BackgroundTexture, SDL_BLENDMODE_BLEND);
    }
}

Item *Component::GetSelectedItem()
{
    return SelectedItem;
}

void Component::TriggerEnterEvent()
{
    EnterRequested = true;
}

void Component::TriggerExitEvent()
{
    ExitRequested = true;
}



void Component::TriggerMenuEnterEvent(int menuIndex)
{
    MenuEnterRequested = true;
    MenuEnterIndex = menuIndex;
}

void Component::TriggerMenuScrollEvent()
{
    MenuScrollRequested = true;
}


void Component::TriggerMenuExitEvent(int menuIndex)
{
    MenuExitRequested = true;
    MenuExitIndex = menuIndex;
}
void Component::TriggerHighlightEvent(Item *selectedItem)
{
    NewItemSelected = true;
    this->SelectedItem = selectedItem;
}


bool Component::IsIdle()
{
    return (CurrentAnimationState == IDLE);
}

bool Component::IsHidden()
{
    return (CurrentAnimationState == HIDDEN);
}
bool Component::IsWaiting()
{
    return (CurrentAnimationState == HIGHLIGHT_WAIT);
}

bool Component::IsMenuScrolling()
{
    return (CurrentAnimationState == MENU_ENTER || CurrentAnimationState == MENU_SCROLL || CurrentAnimationState == MENU_EXIT || MenuScrollRequested);
}


std::string Component::GetCollectionName()
{
    return CollectionName;
}

void Component::SetCollectionName(std::string collectionName)
{
    CollectionName = collectionName;
}

TweenSets &Component::GetTweens()
{
    return Tweens;
}

void Component::ImportTweens(TweenSets &set)
{
    Tweens = set;
    CurrentAnimationState = IDLE;
    CurrentTweenIndex = 0;
    CurrentTweenComplete = false;
    ElapsedTweenTime = 0;
}

ViewInfo *Component::GetBaseViewInfo()
{
    return &BaseViewInfo;
}
void Component::UpdateBaseViewInfo(ViewInfo &info)
{
    BaseViewInfo = info;
}

bool Component::IsScrollActive() const
{
    return ScrollActive;
}

void Component::SetScrollActive(bool scrollActive)
{
    ScrollActive = scrollActive;
}


void Component::Update(float dt)
{
    ElapsedTweenTime += dt;
    HighlightExitComplete = false;
    if(IsHidden() || IsWaiting() || (IsIdle() && ExitRequested))
    {
        CurrentTweenComplete = true;
    }

    if(CurrentTweenComplete)
    {
        CurrentTweens = NULL;

        // There was no request to override our state path. Continue on as normal.
        std::stringstream ss;
        switch(CurrentAnimationState)
        {
        case MENU_ENTER:
            CurrentTweens = NULL;
            CurrentAnimationState = IDLE;
            break;

        case MENU_SCROLL:
            CurrentTweens = NULL;
            CurrentAnimationState = IDLE;
            break;

        case MENU_EXIT:
            CurrentTweens = NULL;
            CurrentAnimationState = IDLE;
            break;


        case ENTER:
            CurrentTweens = Tweens.GetTween("enter", MenuEnterIndex);
            CurrentAnimationState = HIGHLIGHT_ENTER;
            break;

        case EXIT:
            CurrentTweens = NULL;
            CurrentAnimationState = HIDDEN;
            break;

        case HIGHLIGHT_ENTER:
            CurrentTweens = Tweens.GetTween("idle", MenuEnterIndex);
            CurrentAnimationState = IDLE;
            break;

        case IDLE:
            // prevent us from automatically jumping to the exit tween upon enter
            if(EnterRequested)
            {
                EnterRequested = false;
                NewItemSelected = false;
            }
            else if(MenuExitRequested && (!MenuEnterRequested || MenuExitRequested <= MenuEnterRequested))
            {
                CurrentTweens = Tweens.GetTween("menuExit", MenuExitIndex);
                CurrentAnimationState = MENU_EXIT;
                MenuExitRequested = false;
            }
            else if(MenuEnterRequested && (!MenuExitRequested || MenuExitRequested > MenuEnterRequested))
            {
                CurrentTweens = Tweens.GetTween("menuEnter", MenuEnterIndex);
                CurrentAnimationState = MENU_ENTER;
                MenuEnterRequested = false;

            }
            else if(MenuScrollRequested)
            {
                MenuScrollRequested = false;
                CurrentTweens = Tweens.GetTween("menuScroll", MenuEnterIndex);
                CurrentAnimationState = MENU_SCROLL;
            }
            else if(IsScrollActive() || NewItemSelected || ExitRequested)
            {
                CurrentTweens = Tweens.GetTween("highlightExit", MenuEnterIndex);
                CurrentAnimationState = HIGHLIGHT_EXIT;
            }
            else
            {
                CurrentTweens = Tweens.GetTween("idle", MenuEnterIndex);
                CurrentAnimationState = IDLE;
            }
            break;

        case HIGHLIGHT_EXIT:

            // intentionally break down
        case HIGHLIGHT_WAIT:

            if(ExitRequested && (CurrentAnimationState == HIGHLIGHT_WAIT))
            {
                CurrentTweens = Tweens.GetTween("highlightExit", MenuEnterIndex);
                CurrentAnimationState = HIGHLIGHT_EXIT;

            }
            else if(ExitRequested && (CurrentAnimationState == HIGHLIGHT_EXIT))
            {

                CurrentTweens = Tweens.GetTween("exit", MenuEnterIndex);
                CurrentAnimationState = EXIT;
                ExitRequested = false;
            }
            else if(IsScrollActive())
            {
                CurrentTweens = NULL;
                CurrentAnimationState = HIGHLIGHT_WAIT;
            }
            else if(NewItemSelected)
            {
                CurrentTweens = Tweens.GetTween("highlightEnter", MenuEnterIndex);
                CurrentAnimationState = HIGHLIGHT_ENTER;
                HighlightExitComplete = true;
                NewItemSelected = false;
            }
            else
            {
                CurrentTweens = NULL;
                CurrentAnimationState = HIGHLIGHT_WAIT;
            }
            break;

        case HIDDEN:
            if(EnterRequested || ExitRequested)
            {
                CurrentTweens = Tweens.GetTween("enter", MenuEnterIndex);
                CurrentAnimationState = ENTER;
            }

            else if(MenuExitRequested && (!MenuEnterRequested || MenuExitRequested <= MenuEnterRequested))
            {
                CurrentTweens = Tweens.GetTween("menuExit", MenuExitIndex);
                CurrentAnimationState = MENU_EXIT;
                MenuExitRequested = false;
            }
            else if(MenuEnterRequested && (!MenuExitRequested || MenuExitRequested > MenuEnterRequested))
            {
                CurrentTweens = Tweens.GetTween("menuEnter", MenuEnterIndex);
                CurrentAnimationState = MENU_ENTER;
                MenuEnterRequested = false;

            }
            else if(MenuScrollRequested)
            {
                MenuScrollRequested = false;
                CurrentTweens = Tweens.GetTween("menuScroll", MenuEnterIndex);
                CurrentAnimationState = MENU_SCROLL;
            }
            else
            {
                CurrentTweens = NULL;
                CurrentAnimationState = HIDDEN;
            }
        }

        CurrentTweenIndex = 0;
        CurrentTweenComplete = false;

        ElapsedTweenTime = 0;
    }

    CurrentTweenComplete = Animate(IsIdle());
}

void Component::Draw()
{

    if(BackgroundTexture)
    {
        ViewInfo *info = GetBaseViewInfo();
        SDL_Rect rect;
        rect.h = static_cast<int>(info->GetHeight());
        rect.w = static_cast<int>(info->GetWidth());
        rect.x = static_cast<int>(info->GetXRelativeToOrigin());
        rect.y = static_cast<int>(info->GetYRelativeToOrigin());


        SDL_SetTextureColorMod(BackgroundTexture,
                               static_cast<char>(info->GetBackgroundRed()*255),
                               static_cast<char>(info->GetBackgroundGreen()*255),
                               static_cast<char>(info->GetBackgroundBlue()*255));

        SDL::RenderCopy(BackgroundTexture, static_cast<char>(info->GetBackgroundAlpha()*255), NULL, &rect, info->GetAngle());
    }
}

bool Component::Animate(bool loop)
{
    bool completeDone = false;
    if(!CurrentTweens || CurrentTweenIndex >= CurrentTweens->size())
    {
        completeDone = true;
    }
    else if(CurrentTweens)
    {
        bool currentDone = true;
        std::vector<Tween> &TweenSets = CurrentTweens->at(CurrentTweenIndex);

        for(unsigned int i = 0; i < TweenSets.size(); i++)
        {
            Tween &tween = TweenSets.at(i);
            float elapsedTime = ElapsedTweenTime;

            //todo: too many levels of nesting
            if(elapsedTime < tween.GetDuration())
            {
                currentDone = false;
            }
            else
            {
                elapsedTime = tween.GetDuration();
            }

            float value = tween.Animate(elapsedTime);

            switch(tween.GetProperty())
            {
            case TWEEN_PROPERTY_X:
                GetBaseViewInfo()->SetX(value);
                break;

            case TWEEN_PROPERTY_Y:
                GetBaseViewInfo()->SetY(value);
                break;

            case TWEEN_PROPERTY_HEIGHT:
                GetBaseViewInfo()->SetHeight(value);
                break;

            case TWEEN_PROPERTY_WIDTH:
                GetBaseViewInfo()->SetWidth(value);
                break;

            case TWEEN_PROPERTY_ANGLE:
                GetBaseViewInfo()->SetAngle(value);
                break;

            case TWEEN_PROPERTY_ALPHA:
                GetBaseViewInfo()->SetAlpha(value);
                break;

            case TWEEN_PROPERTY_X_ORIGIN:
                GetBaseViewInfo()->SetXOrigin(value);
                break;

            case TWEEN_PROPERTY_Y_ORIGIN:
                GetBaseViewInfo()->SetYOrigin(value);
                break;

            case TWEEN_PROPERTY_X_OFFSET:
                GetBaseViewInfo()->SetXOffset(value);
                break;

            case TWEEN_PROPERTY_Y_OFFSET:
                GetBaseViewInfo()->SetYOffset(value);
                break;

            case TWEEN_PROPERTY_FONT_SIZE:
                GetBaseViewInfo()->SetFontSize(value);
                break;

            case TWEEN_PROPERTY_BACKGROUND_ALPHA:
                GetBaseViewInfo()->SetBackgroundAlpha(value);
                break;
            }
        }

        if(currentDone)
        {
            CurrentTweenIndex++;
            ElapsedTweenTime = 0;
        }
    }

    if(!CurrentTweens || CurrentTweenIndex >= CurrentTweens->size())
    {
        if(loop)
        {
            CurrentTweenIndex = 0;
        }
        completeDone = true;
    }

    return completeDone;
}
