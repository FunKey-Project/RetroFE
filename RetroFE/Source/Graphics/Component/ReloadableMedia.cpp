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
    if(loadedComponent_)
    {
        delete loadedComponent_;
        loadedComponent_ = NULL;
    }

    Item *selectedItem = getSelectedItem();
    if(!selectedItem) return;

    config_.getProperty("currentCollection", currentCollection_);

    // build clone list
    std::vector<std::string> names;

    names.push_back(selectedItem->name);
    names.push_back(selectedItem->fullTitle);

    if(selectedItem->cloneof.length() > 0)
    {
        names.push_back(selectedItem->cloneof);
    }

    if(isVideo_)
    {
        for(unsigned int n = 0; n < names.size() && !loadedComponent_; ++n)
        {
            std::string basename = names[n];
            if(systemMode_)
            {
                // only look through the master collection for the system artifact
                loadedComponent_ = findComponent(collectionName, "video", "video", true);
            }
            else
            {
                // check the master collection for the artifact 
                loadedComponent_ = findComponent(collectionName, "video", basename, false);

                if(!loadedComponent_ && selectedItem->collectionInfo->hasSubcollections())
                {
                    if(selectedItem->leaf)
                    {
                        // check the subcollection for artwork artifacts
                        loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, false);
                    }
                    else
                    {
                        // item is a submenu, check the subcollection for system artwork artifacts
                        loadedComponent_ = findComponent(selectedItem->collectionInfo->name, "video", "video", true);
                    }
                }
            }

            if(loadedComponent_)
            {
                loadedComponent_->allocateGraphicsMemory();
                baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
                baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
            }
        }
    }

    // check for images if video could not be found (and was specified)
    for(unsigned int n = 0; n < names.size() && !loadedComponent_; ++n)
    {
        std::string basename = names[n];

        std::string typeLC = Utils::toLower(type_);

        if(typeLC == "numberButtons")
        {
            basename = selectedItem->numberButtons;
        }
        else if(typeLC == "numberPlayers")
        {
            basename = selectedItem->numberPlayers;
        }
        else if(typeLC == "year")
        {
            basename = selectedItem->year;
        }
        else if(typeLC == "title")
        {
            basename = selectedItem->title;
        }
        else if(typeLC == "manufacturer")
        {
            basename = selectedItem->manufacturer;
        }
        else if(typeLC == "genre")
        {
            basename = selectedItem->genre;
        }

        Utils::replaceSlashesWithUnderscores(basename);

        if(systemMode_)
        {
            // only look through the master collection for the system artifact
            loadedComponent_ = findComponent(collectionName, type_, type_, true);
        }
        else
        {
            // check the master collection for the artifact 
            loadedComponent_ = findComponent(collectionName, type_, basename, false);

            if(!loadedComponent_ && selectedItem->collectionInfo->hasSubcollections())
            {
                if(selectedItem->leaf)
                {
                    // check the subcollection for artwork artifacts
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, false);
                }
                else
                {
                    // item is a submenu, check the subcollection for system artwork artifacts
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, true);
                }
            }
        }
      

        if (loadedComponent_ != NULL)
        {
             loadedComponent_->allocateGraphicsMemory();
             baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
             baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
        }
    }

    // if image and artwork was not specified, fall back to displaying text
    if(!loadedComponent_ && textFallback_)
    {
        loadedComponent_ = new Text(selectedItem->fullTitle, FfntInst_, scaleX_, scaleY_);
        baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
        baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
    }
}


Component *ReloadableMedia::findComponent(std::string collection, std::string type, std::string basename, bool systemMode)
{
    std::string imagePath;
    Component *component = NULL;
    VideoBuilder videoBuild;
    ImageBuilder imageBuild;

    // check the system folder
    config_.getMediaPropertyAbsolutePath(collection, type, systemMode, imagePath);
std::cout << "searching path: " << imagePath << "  =>" << basename << std::endl;

    if(type == "video")
    {
        component = videoBuild.createVideo(imagePath, basename, scaleX_, scaleY_);
    }
    else
    {
        component = imageBuild.CreateImage(imagePath, basename, scaleX_, scaleY_);
    }

    return component;

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
