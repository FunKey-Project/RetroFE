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
#include "Image.h"
#include "../../SDL.h"
#include <SDL2/SDL_image.h>

Image::Image(std::string file)
    : texture_(NULL)
    , file_(file)
{
}

Image::~Image()
{
}

void Image::Initialize()
{
    if(!texture_)
    {
        SDL_LockMutex(SDL::getMutex());
        texture_ = IMG_LoadTexture(SDL::getRenderer(), file_.c_str());

        if (texture_ != NULL)
        {
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(texture_, NULL, NULL, &info.width, &info.height);
        }
        SDL_UnlockMutex(SDL::getMutex());
    }
}

void Image::DeInitialize()
{
    if(texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = NULL;
    }
}

void Image::update(float dt)
{
    Component::update(dt);

    if(!texture_)
    {
        SDL_LockMutex(SDL::getMutex());
        texture_ = IMG_LoadTexture(SDL::getRenderer(), file_.c_str());

        if (texture_ != NULL)
        {
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(texture_, NULL, NULL, &info.width, &info.height);
        }
        SDL_UnlockMutex(SDL::getMutex());
    }
}

void Image::draw()
{
    if(texture_)
    {
        SDL_Rect rect;

        rect.x = info.x;
        rect.y = info.y;
        rect.h = info.width;
        rect.w = info.height;

        SDL::renderCopy(texture_, (unsigned char)(info.alpha*255), NULL, &rect, 45);
    }
}
