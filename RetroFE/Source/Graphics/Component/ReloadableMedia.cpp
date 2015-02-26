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

#include "ReloadableMedia.h"
#include "ImageBuilder.h"
#include "VideoBuilder.h"
#include "ReloadableText.h"
#include "../ViewInfo.h"
#include "../../Video/VideoFactory.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../Utility/Utils.h"
#include "../../SDL.h"
#include <fstream>
#include <vector>
#include <iostream>

ReloadableMedia::ReloadableMedia(Configuration &config, std::string type, bool isVideo, Font *font, SDL_Color fontColor, float scaleX, float scaleY)
    : Config(config)
    , LoadedComponent(NULL)
    , ReloadRequested(false)
    , FirstLoad(true)
    , Type(type)
    , IsVideo(isVideo)
    , FontInst(font)
    , FontColor(fontColor)
    , TextFallback(false)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
{
    AllocateGraphicsMemory();
}

ReloadableMedia::~ReloadableMedia()
{
    if (LoadedComponent != NULL)
    {
        delete LoadedComponent;
    }
}

void ReloadableMedia::EnableTextFallback(bool value)
{
    TextFallback = value;
}

void ReloadableMedia::Update(float dt)
{
    if(NewItemSelected)
    {
        ReloadRequested = true;
    }
    // wait for the right moment to reload the image
    if (ReloadRequested && (HighlightExitComplete || FirstLoad))
    {

        ReloadTexture();
        ReloadRequested = false;
        FirstLoad = false;
    }

    if(LoadedComponent)
    {

        // video needs to run a frame to start getting size info
        if(GetBaseViewInfo()->GetImageHeight() == 0 && GetBaseViewInfo()->GetImageWidth() == 0)
        {
            GetBaseViewInfo()->SetImageWidth(LoadedComponent->GetBaseViewInfo()->GetImageWidth());
            GetBaseViewInfo()->SetImageHeight(LoadedComponent->GetBaseViewInfo()->GetImageHeight());
        }

        LoadedComponent->Update(dt);
    }

    // needs to be ran at the end to prevent the NewItemSelected flag from being detected
    Component::Update(dt);

}

void ReloadableMedia::AllocateGraphicsMemory()
{
    FirstLoad = true;

    if(LoadedComponent)
    {
        LoadedComponent->AllocateGraphicsMemory();
    }

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::AllocateGraphicsMemory();
}

void ReloadableMedia::LaunchEnter()
{
    if(LoadedComponent)
    {
        LoadedComponent->LaunchEnter();
    }
}

void ReloadableMedia::LaunchExit()
{
    if(LoadedComponent)
    {
        LoadedComponent->LaunchExit();
    }
}

void ReloadableMedia::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();

    if(LoadedComponent)
    {
        LoadedComponent->FreeGraphicsMemory();
    }
}
void ReloadableMedia::ReloadTexture()
{
    bool found = false;

    if(LoadedComponent)
    {
        delete LoadedComponent;
        LoadedComponent = NULL;
    }

    Item *selectedItem = GetSelectedItem();

    if (selectedItem != NULL)
    {
        if(IsVideo)
        {
            std::vector<std::string> names;

            names.push_back(selectedItem->GetName());

            if(selectedItem->GetCloneOf().length() > 0)
            {
                names.push_back(selectedItem->GetCloneOf());
            }
            std::string videoPath;
            Config.GetMediaPropertyAbsolutePath(GetCollectionName(), "video", videoPath);

            for(unsigned int n = 0; n < names.size() && !found; ++n)
            {
                VideoBuilder videoBuild;

                LoadedComponent = videoBuild.CreateVideo(videoPath, names[n], ScaleX, ScaleY);

                if(LoadedComponent)
                {
                    LoadedComponent->AllocateGraphicsMemory();
                    GetBaseViewInfo()->SetImageWidth(LoadedComponent->GetBaseViewInfo()->GetImageWidth());
                    GetBaseViewInfo()->SetImageHeight(LoadedComponent->GetBaseViewInfo()->GetImageHeight());
                    found = true;
                }
            }
        }

        std::string imageBasename = selectedItem->GetFullTitle();

        std::string typeLC = Utils::ToLower(Type);

        if(typeLC == "numberButtons")
        {
            imageBasename = selectedItem->GetNumberButtons();
        }
        else if(typeLC == "numberPlayers")
        {
            imageBasename = selectedItem->GetNumberPlayers();
        }
        else if(typeLC == "year")
        {
            imageBasename = selectedItem->GetYear();
        }
        else if(typeLC == "title")
        {
            imageBasename = selectedItem->GetTitle();
        }
        else if(typeLC == "manufacturer")
        {
            imageBasename = selectedItem->GetManufacturer();
        }

        if(!LoadedComponent)
        {
            std::string imagePath;
            Config.GetMediaPropertyAbsolutePath(GetCollectionName(), Type, false, imagePath);

            ImageBuilder imageBuild;

            LoadedComponent = imageBuild.CreateImage(imagePath, imageBasename, ScaleX, ScaleY);

            if(!LoadedComponent)
            {
                 Config.GetMediaPropertyAbsolutePath(imageBasename, Type, true, imagePath);
                 LoadedComponent = imageBuild.CreateImage(imagePath, Type, ScaleX, ScaleY);
            }

            if (LoadedComponent != NULL)
            {
                 LoadedComponent->AllocateGraphicsMemory();
                 GetBaseViewInfo()->SetImageWidth(LoadedComponent->GetBaseViewInfo()->GetImageWidth());
                 GetBaseViewInfo()->SetImageHeight(LoadedComponent->GetBaseViewInfo()->GetImageHeight());
            }

        }

        if(!LoadedComponent && TextFallback)
        {
            LoadedComponent = new Text(imageBasename, FontInst, FontColor, ScaleX, ScaleY);
            GetBaseViewInfo()->SetImageWidth(LoadedComponent->GetBaseViewInfo()->GetImageWidth());
            GetBaseViewInfo()->SetImageHeight(LoadedComponent->GetBaseViewInfo()->GetImageHeight());
        }
    }
}

void ReloadableMedia::Draw()
{
    ViewInfo *info = GetBaseViewInfo();

    Component::Draw();

    if(LoadedComponent)
    {
        info->SetImageHeight(LoadedComponent->GetBaseViewInfo()->GetImageHeight());
        info->SetImageWidth(LoadedComponent->GetBaseViewInfo()->GetImageWidth());
        LoadedComponent->UpdateBaseViewInfo(*info);
        LoadedComponent->Draw();
    }
}
