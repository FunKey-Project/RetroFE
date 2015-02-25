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

#include "VideoComponent.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"

VideoComponent::VideoComponent(IVideo *videoInst, std::string videoFile, float scaleX, float scaleY)
    : VideoFile(videoFile)
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

        // video needs to run a frame to start getting size info
        if(GetBaseViewInfo()->GetImageHeight() == 0 && GetBaseViewInfo()->GetImageWidth() == 0)
        {
            GetBaseViewInfo()->SetImageHeight(static_cast<float>(VideoInst->GetHeight()));
            GetBaseViewInfo()->SetImageWidth(static_cast<float>(VideoInst->GetWidth()));
        }
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

    Component::FreeGraphicsMemory();
}

void VideoComponent::LaunchEnter()
{
    FreeGraphicsMemory();
}
void VideoComponent::LaunchExit()
{
    AllocateGraphicsMemory();
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
        SDL::RenderCopy(texture, static_cast<int>(info->GetAlpha() * 255), NULL, &rect, static_cast<int>(info->GetAngle()));
    }
}
