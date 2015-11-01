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

#include <cstdlib>

AttractMode::AttractMode()
    : idleTime(0)
    , isActive_(false)
    , elapsedTime_(0)
    , activeTime_(0)
{
}

void AttractMode::reset()
{
    elapsedTime_ = 0;
    isActive_ = false;
    activeTime_ = 0;
}

void AttractMode::update(float dt)
{
    elapsedTime_ += dt;

    // enable attract mode when idling for the expected time. Disable if idle time is set to 0.
    if(!isActive_ && elapsedTime_ > idleTime && idleTime > 0)
    {
        isActive_ = true;
        elapsedTime_ = 0;
        activeTime_ = ((float)((1000+rand()) % 5000)) / 1000;
    }

    if(isActive_)
    {
        //todo: enable scrolling

        if(elapsedTime_ > activeTime_)
        {
            elapsedTime_ = 0;
            isActive_ = false;
            //todo: go to idle
        }
    }
}
