/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "AttractMode.h"
#include "../Graphics/Page.h"

AttractMode::AttractMode()
: IsActive(false)
, ElapsedTime(0)
, ActiveTime(0)
, IdleTime(0)
{
}

void AttractMode::SetIdleTime(float time)
{
    IdleTime = time;
}
void AttractMode::Reset()
{
    ElapsedTime = 0;
    IsActive = false;
    ActiveTime = 0;
}

void AttractMode::Update(float dt, Page &page)
{
    ElapsedTime += dt;

    // enable attract mode when idling for the expected time. Disable if idle time is set to 0.
    if(!IsActive && ElapsedTime > IdleTime && IdleTime > 0)
    {
        IsActive = true;
        ElapsedTime = 0;
        ActiveTime = ((float)((1000+rand()) % 5000)) / 1000;
    }

    if(IsActive)
    {
        page.SetScrolling(Page::ScrollDirectionForward);

        if(ElapsedTime > ActiveTime)
        {
            ElapsedTime = 0;
            IsActive = false;
            page.SetScrolling(Page::ScrollDirectionIdle);
        }
    }
}