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
#include "NullVideo.h"
#include "../Utility/Log.h"
#include "../SDL.h"
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <SDL2/SDL.h>

NullVideo::NullVideo()
{
}
NullVideo::~NullVideo()
{
}

void NullVideo::SetNumLoops(int n)
{
}

SDL_Texture *NullVideo::GetTexture() const
{
    return NULL;
}

bool NullVideo::Initialize()
{
    return true;
}

bool NullVideo::DeInitialize()
{
    return true;
}

bool NullVideo::Stop()
{
    return true;
}

bool NullVideo::Play(std::string file)
{
    return true;
}

int NullVideo::GetHeight()
{
    return 0;
}

int NullVideo::GetWidth()
{
    return 0;
}


void NullVideo::Draw()
{
}

void NullVideo::Update(float dt)
{
}

