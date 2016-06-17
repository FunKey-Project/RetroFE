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
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL2/SDL_image.h>

Image::Image(std::string file, Page &p, float scaleX, float scaleY)
    : Component(p)
    , texture_(NULL)
    , file_(file)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
{
    allocateGraphicsMemory();
}

Image::~Image()
{
    freeGraphicsMemory();
}

void Image::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        SDL_DestroyTexture(texture_);
        texture_ = NULL;
    }
    SDL_UnlockMutex(SDL::getMutex());
}

void Image::allocateGraphicsMemory()
{
    int width;
    int height;

    Component::allocateGraphicsMemory();

    if(!texture_)
    {
        SDL_LockMutex(SDL::getMutex());
        texture_ = IMG_LoadTexture(SDL::getRenderer(), file_.c_str());

        if (texture_ != NULL)
        {
            SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(texture_, NULL, NULL, &width, &height);
            baseViewInfo.ImageWidth = width * scaleX_;
            baseViewInfo.ImageHeight = height * scaleY_;
        }
        SDL_UnlockMutex(SDL::getMutex());

    }
}

void Image::draw()
{
    Component::draw();

    if(texture_)
    {
        SDL_Rect rect;

        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

        SDL::renderCopy(texture_, static_cast<char>((baseViewInfo.Alpha * 255)), NULL, &rect, baseViewInfo.Angle, baseViewInfo.Reflection, baseViewInfo.ReflectionDistance, baseViewInfo.ReflectionScale, static_cast<char>(baseViewInfo.ReflectionAlpha*255));
    }
}
