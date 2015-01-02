/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "VideoComponent.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"

VideoComponent::VideoComponent(IVideo *videoInst, std::string videoFile, float scaleX, float scaleY)
    : VideoTexture(NULL)
    , VideoFile(videoFile)
    , VideoInst(videoInst)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
    , IsPlaying(false)
{
//   AllocateGraphicsMemory();
}

VideoComponent::~VideoComponent()
{
    FreeGraphicsMemory();

    if(VideoInst)
    {
        VideoInst->Stop();
    }
}

void VideoComponent::Update(float dt)
{
    if(IsPlaying)
    {
        VideoInst->Update(dt);
    }

    Component::Update(dt);

}

void VideoComponent::AllocateGraphicsMemory()
{
    Component::AllocateGraphicsMemory();

    if(!IsPlaying)
    {
        IsPlaying = VideoInst->Play(VideoFile);
    }
}

void VideoComponent::FreeGraphicsMemory()
{
    VideoInst->Stop();
    IsPlaying = false;

    if (VideoTexture != NULL)
    {
        SDL_LockMutex(SDL::GetMutex());
        SDL_DestroyTexture(VideoTexture);
        SDL_UnlockMutex(SDL::GetMutex());
    }

    Component::FreeGraphicsMemory();
}

void VideoComponent::Draw()
{
    ViewInfo *info = GetBaseViewInfo();
    SDL_Rect rect;

    rect.x = static_cast<int>(info->GetXRelativeToOrigin());
    rect.y = static_cast<int>(info->GetYRelativeToOrigin());
    rect.h = static_cast<int>(info->GetHeight());
    rect.w = static_cast<int>(info->GetWidth());

    VideoInst->Draw();
    SDL_Texture *texture = VideoInst->GetTexture();

    if(texture)
    {
        SDL::RenderCopy(texture, static_cast<int>(info->GetAlpha() * 255), NULL, &rect, info->GetAngle());
    }
}
