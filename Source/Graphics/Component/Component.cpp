/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Component.h"
#include "../Animate/Tween.h"
#include "../../Graphics/ViewInfo.h"
#include "../../Utility/Log.h"

Component::Component()
{
    OnEnterTweens = NULL;
    OnExitTweens = NULL;
    OnIdleTweens = NULL;
    OnHighlightEnterTweens = NULL;
    OnHighlightExitTweens = NULL;
    SelectedItem = NULL;
    NewItemSelectedSinceEnter = false;
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
    NewItemSelected = false;
    HighlightExitComplete = false;
    CurrentTweens = NULL;
    CurrentTweenIndex = 0;
    CurrentTweenComplete = false;
    ElapsedTweenTime =0;
    ScrollActive = false;
}
void Component::AllocateGraphicsMemory()
{
}

void Component::TriggerEnterEvent()
{
    EnterRequested = true;
}

void Component::TriggerExitEvent()
{
    ExitRequested = true;
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
        switch(CurrentAnimationState)
        {
        case ENTER:
            CurrentTweens = OnHighlightEnterTweens;
            CurrentAnimationState = HIGHLIGHT_ENTER;
            break;

        case EXIT:
            CurrentTweens = NULL;
            CurrentAnimationState = HIDDEN;

            break;

        case HIGHLIGHT_ENTER:
            CurrentTweens = OnIdleTweens;
            CurrentAnimationState = IDLE;
            break;

        case IDLE:
            // prevent us from automatically jumping to the exit tween upon enter
            if(EnterRequested)
            {
                EnterRequested = false;
                NewItemSelected = false;
            }
            else if(IsScrollActive() || NewItemSelected || ExitRequested)
            {
                CurrentTweens = OnHighlightExitTweens;
                CurrentAnimationState = HIGHLIGHT_EXIT;
            }
            else
            {
                CurrentTweens = OnIdleTweens;
                CurrentAnimationState = IDLE;
            }
            break;

        case HIGHLIGHT_EXIT:

        // intentionally break down
        case HIGHLIGHT_WAIT:

            if(ExitRequested && (CurrentAnimationState == HIGHLIGHT_WAIT))
            {
                CurrentTweens = OnHighlightExitTweens;
                CurrentAnimationState = HIGHLIGHT_EXIT;

            }
            else if(ExitRequested && (CurrentAnimationState == HIGHLIGHT_EXIT))
            {

                CurrentTweens = OnExitTweens;
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
                CurrentTweens = OnHighlightEnterTweens;
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
                CurrentTweens = OnEnterTweens;
                CurrentAnimationState = ENTER;
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
        std::vector<Tween *> *tweenSet = CurrentTweens->at(CurrentTweenIndex);

        for(unsigned int i = 0; i < tweenSet->size(); i++)
        {
            Tween *tween = tweenSet->at(i);
            float elapsedTime = ElapsedTweenTime;

            //todo: too many levels of nesting
            if(elapsedTime < tween->GetDuration())
            {
                currentDone = false;
            }
            else
            {
                elapsedTime = tween->GetDuration();
            }

            float value = tween->Animate(elapsedTime);

            switch(tween->GetProperty())
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

            case TWEEN_PROPERTY_TRANSPARENCY:
                GetBaseViewInfo()->SetTransparency(value);
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
