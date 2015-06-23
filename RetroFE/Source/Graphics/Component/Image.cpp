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

Image::Image(std::string file, float scaleX, float scaleY)
    : Texture(NULL)
    , File(file)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
{
    AllocateGraphicsMemory();
}

Image::~Image()
{
    FreeGraphicsMemory();
}

void Image::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();

    SDL_LockMutex(SDL::GetMutex());
    if (Texture != NULL)
    {
        SDL_DestroyTexture(Texture);
        Texture = NULL;
    }
    SDL_UnlockMutex(SDL::GetMutex());
}

void Image::AllocateGraphicsMemory()
{
    int width;
    int height;

    Component::AllocateGraphicsMemory();

    if(!Texture)
    {
        SDL_LockMutex(SDL::GetMutex());
        Texture = IMG_LoadTexture(SDL::GetRenderer(), File.c_str());

        if (Texture != NULL)
        {
            SDL_SetTextureBlendMode(Texture, SDL_BLENDMODE_BLEND);
            SDL_QueryTexture(Texture, NULL, NULL, &width, &height);
            BaseViewInfo.ImageWidth = width * ScaleX;
            BaseViewInfo.ImageHeight = height * ScaleY;
        }
        SDL_UnlockMutex(SDL::GetMutex());

    }
}

void Image::Draw()
{
    Component::Draw();

    if(Texture)
    {
        SDL_Rect rect;

        rect.x = static_cast<int>(BaseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(BaseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(BaseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(BaseViewInfo.ScaledWidth());

        SDL::RenderCopy(Texture, static_cast<char>((BaseViewInfo.Alpha * 255)), NULL, &rect, BaseViewInfo.Angle);
    }
}
