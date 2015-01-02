/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "ReloadableMedia.h"
#include "ImageBuilder.h"
#include "VideoBuilder.h"
#include "../ViewInfo.h"
#include "../../Video/VideoFactory.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../Utility/Utils.h"
#include "../../SDL.h"
#include <fstream>
#include <vector>
#include <iostream>

ReloadableMedia::ReloadableMedia(std::string imagePath, std::string videoPath, bool isVideo, float scaleX, float scaleY)
    : LoadedComponent(NULL)
    , ImagePath(imagePath)
    , VideoPath(videoPath)
    , ReloadRequested(false)
    , FirstLoad(true)
    , IsVideo(isVideo)
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

            for(unsigned int n = 0; n < names.size() && !found; ++n)
            {
                std::string filePrefix;
                filePrefix.append(VideoPath);
                filePrefix.append("/");
                filePrefix.append(names[n]);

                std::string file;

                VideoBuilder videoBuild;

                LoadedComponent = videoBuild.CreateVideo(VideoPath, names[n], ScaleX, ScaleY);

                if(LoadedComponent)
                {
                    LoadedComponent->AllocateGraphicsMemory();
                    found = true;
                }
            }
        }

        if(!LoadedComponent)
        {
            ImageBuilder imageBuild;
            LoadedComponent = imageBuild.CreateImage(ImagePath, selectedItem->GetFullTitle(), ScaleX, ScaleY);

            if (LoadedComponent != NULL)
            {
                LoadedComponent->AllocateGraphicsMemory();
                GetBaseViewInfo()->SetImageWidth(LoadedComponent->GetBaseViewInfo()->GetImageWidth());
                GetBaseViewInfo()->SetImageHeight(LoadedComponent->GetBaseViewInfo()->GetImageHeight());
            }
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
