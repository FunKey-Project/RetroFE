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