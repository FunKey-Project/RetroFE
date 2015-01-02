/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
            GetBaseViewInfo()->SetImageWidth(width * ScaleX);
            GetBaseViewInfo()->SetImageHeight(height * ScaleY);
        }
        SDL_UnlockMutex(SDL::GetMutex());

    }
}

void Image::Draw()
{
    Component::Draw();

    if(Texture)
    {
        ViewInfo *info = GetBaseViewInfo();
        SDL_Rect rect;

        rect.x = static_cast<int>(info->GetXRelativeToOrigin());
        rect.y = static_cast<int>(info->GetYRelativeToOrigin());
        rect.h = static_cast<int>(info->GetHeight());
        rect.w = static_cast<int>(info->GetWidth());

        SDL::RenderCopy(Texture, static_cast<char>((info->GetAlpha() * 255)), NULL, &rect, info->GetAngle());
    }
}
