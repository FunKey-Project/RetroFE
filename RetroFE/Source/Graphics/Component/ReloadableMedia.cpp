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

ReloadableMedia::ReloadableMedia(Configuration &config, bool systemMode, std::string type, bool isVideo, Font *font, float scaleX, float scaleY)
    : config_(config)
    , systemMode_(systemMode)
    , loadedComponent_(NULL)
    , reloadRequested_(false)
    , firstLoad_(true)
    , videoInst_(NULL)
    , isVideo_(isVideo)
    , FfntInst_(font)
    , textFallback_(false)
    , type_(type)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
{
    allocateGraphicsMemory();
}

ReloadableMedia::~ReloadableMedia()
{
    if (loadedComponent_ != NULL)
    {
        delete loadedComponent_;
    }
}

void ReloadableMedia::enableTextFallback_(bool value)
{
    textFallback_ = value;
}

void ReloadableMedia::update(float dt)
{
    if(newItemSelected)
    {
    	std::string collection;
    	config_.getProperty("currentCollection", collection);

        if(!systemMode_ || (systemMode_ && currentCollection_ != collection))
        {
            reloadRequested_ = true;
        }
    }
    // wait for the right moment to reload the image
    if (reloadRequested_ && (highlightExitComplete || firstLoad_))
    {

        reloadTexture();
        reloadRequested_ = false;
        firstLoad_ = false;
    }

    if(loadedComponent_)
    {

        // video needs to run a frame to start getting size info
        if(baseViewInfo.ImageHeight == 0 && baseViewInfo.ImageWidth == 0)
        {
            baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
            baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
        }

        loadedComponent_->update(dt);
    }

    // needs to be ran at the end to prevent the NewItemSelected flag from being detected
    Component::update(dt);

}

void ReloadableMedia::allocateGraphicsMemory()
{
    firstLoad_ = true;

    if(loadedComponent_)
    {
        loadedComponent_->allocateGraphicsMemory();
    }

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::allocateGraphicsMemory();
}

void ReloadableMedia::launchEnter()
{
    if(loadedComponent_)
    {
        loadedComponent_->launchEnter();
    }
}

void ReloadableMedia::launchExit()
{
    if(loadedComponent_)
    {
        loadedComponent_->launchExit();
    }
}

void ReloadableMedia::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    if(loadedComponent_)
    {
        loadedComponent_->freeGraphicsMemory();
    }
}
void ReloadableMedia::reloadTexture()
{
    bool found = false;

    if(loadedComponent_)
    {
        delete loadedComponent_;
        loadedComponent_ = NULL;
    }

    Item *selectedItem = getSelectedItem();

    config_.getProperty("currentCollection", currentCollection_);

    if (selectedItem != NULL)
    {
        std::vector<std::string> names;

        names.push_back(selectedItem->name);
        names.push_back(selectedItem->fullTitle);

        if(selectedItem->cloneof.length() > 0)
        {
            names.push_back(selectedItem->cloneof);
        }

        for(unsigned int n = 0; n < names.size() && !found; ++n)
        {
            if(isVideo_)
            {
                VideoBuilder videoBuild;
                std::string videoPath;

                if(systemMode_)
                {
                    config_.getMediaPropertyAbsolutePath(collectionName, "video", true, videoPath);
                    loadedComponent_ = videoBuild.createVideo(videoPath, "video", scaleX_, scaleY_);

                    if(!loadedComponent_)
                    {
                        config_.getMediaPropertyAbsolutePath(selectedItem->collectionInfo->name, "video", false, videoPath);
                        loadedComponent_ = videoBuild.createVideo(videoPath, names[n], scaleX_, scaleY_);
                    }
                }

                if(!loadedComponent_ && !systemMode_)
                {
                        config_.getMediaPropertyAbsolutePath(names[n], type_, true, videoPath);
                        loadedComponent_ = videoBuild.createVideo(videoPath, "video", scaleX_, scaleY_);
                }

                if(loadedComponent_)
                {
                    loadedComponent_->allocateGraphicsMemory();
                    baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
                    baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
                    found = true;
                }
            }

            std::string imageBasename = names[n];

            std::string typeLC = Utils::toLower(type_);

            if(typeLC == "numberButtons")
            {
                imageBasename = selectedItem->numberButtons;
            }
            else if(typeLC == "numberPlayers")
            {
                imageBasename = selectedItem->numberPlayers;
            }
            else if(typeLC == "year")
            {
                imageBasename = selectedItem->year;
            }
            else if(typeLC == "title")
            {
                imageBasename = selectedItem->title;
            }
            else if(typeLC == "manufacturer")
            {
                imageBasename = selectedItem->manufacturer;
            }
            else if(typeLC == "genre")
            {
                imageBasename = selectedItem->genre;
            }

            Utils::replaceSlashesWithUnderscores(imageBasename);

            if(!loadedComponent_)
            {
                std::string imagePath;

                ImageBuilder imageBuild;

                if(systemMode_)
                {
                    config_.getMediaPropertyAbsolutePath(collectionName, type_, true, imagePath);
                    loadedComponent_ = imageBuild.CreateImage(imagePath, type_, scaleX_, scaleY_);
                }
                else
                {
                    config_.getMediaPropertyAbsolutePath(selectedItem->collectionInfo->name, type_, false, imagePath);
                    loadedComponent_ = imageBuild.CreateImage(imagePath, imageBasename, scaleX_, scaleY_);
                }

                if(!loadedComponent_ && !systemMode_)
                {
                     config_.getMediaPropertyAbsolutePath(imageBasename, type_, true, imagePath);
                     loadedComponent_ = imageBuild.CreateImage(imagePath, type_, scaleX_, scaleY_);
                }

                if (loadedComponent_ != NULL)
                {
                     loadedComponent_->allocateGraphicsMemory();
                     baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
                     baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
                }

            }

            if(!loadedComponent_ && textFallback_)
            {
                loadedComponent_ = new Text(imageBasename, FfntInst_, scaleX_, scaleY_);
                baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
                baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
            }
        }
    }
}

void ReloadableMedia::draw()
{
    Component::draw();

    if(loadedComponent_)
    {
    	baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
    	baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
        loadedComponent_->baseViewInfo = baseViewInfo;
        loadedComponent_->draw();
    }
}
