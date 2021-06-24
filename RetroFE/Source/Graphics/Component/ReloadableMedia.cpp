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

ReloadableMedia::ReloadableMedia(Configuration &config, bool systemMode, bool layoutMode, bool commonMode, bool menuMode, std::string type, Page &p, int displayOffset, bool isVideo, Font *font, float scaleX, float scaleY, bool dithering)
    : Component(p)
    , config_(config)
    , systemMode_(systemMode)
    , layoutMode_(layoutMode)
    , commonMode_(commonMode)
    , menuMode_(menuMode)
    , loadedComponent_(NULL)
    , videoInst_(NULL)
    , isVideo_(isVideo)
    , FfntInst_(font)
    , ditheringAuthorized_(dithering)
    , textFallback_(false)
    , imageFallback_(false)
    , imageAndTextPadding_(0)
    , imageAndText_(false)
    , type_(type)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
    , displayOffset_(displayOffset)

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

void ReloadableMedia::enableImageAndText_(bool value)
{
    imageAndText_ = value;
}

void ReloadableMedia::setImageAndTextPadding_(float value)
{
    imageAndTextPadding_ = value;
}

void ReloadableMedia::enableTextFallback_(bool value)
{
    textFallback_ = value;
}

void ReloadableMedia::enableImageFallback_(bool value)
{
    imageFallback_ = value;
}

void ReloadableMedia::update(float dt)
{

    // Reload media
    if (newItemSelected)
    {
        reloadTexture();
        newItemSelected       = false;
    }
    else if(newScrollItemSelected && getMenuScrollReload())
    {
        reloadTexture(true);
	newScrollItemSelected = false;
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
    if(loadedComponent_)
    {
        loadedComponent_->allocateGraphicsMemory();
    }

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::allocateGraphicsMemory();
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
	reloadTexture(false);
}

void ReloadableMedia::reloadTexture( bool previousItem )
{
    if(loadedComponent_)
    {
        delete loadedComponent_;
        loadedComponent_ = NULL;
    }

    /* Select item to reload */
    Item *selectedItem = NULL;
    if(previousItem){
        selectedItem = page.getPreviousSelectedItem(displayOffset_);
    }
    else{
        selectedItem = page.getSelectedItem(displayOffset_);
    }
    if(!selectedItem) return;

    config_.getProperty("currentCollection", currentCollection_);

    // build clone list
    std::vector<std::string> names;

    names.push_back(selectedItem->name);
    names.push_back(selectedItem->fullTitle);
    names.push_back("default");

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

                // check the master collection for the system artifact 
                loadedComponent_ = findComponent(collectionName, "video", "video", "", true);

                // check the collection for the system artifact
                if(!loadedComponent_)
                {
                  loadedComponent_ = findComponent(selectedItem->collectionInfo->name, "video", "video", "", true);
                }

            }
            else
            {

                // are we looking at a leaf or a submenu
                if (selectedItem->leaf) // item is a leaf
                {

                  // check the master collection for the artifact 
                  loadedComponent_ = findComponent(collectionName, "video", basename, "", false);

                  // check the collection for the artifact
                  if(!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, "video", basename, "", false);
                  }

                  // check the rom directory for the artifact
                  if(!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, "video", "video", selectedItem->filepath, false);
                  }

                }
                else // item is a submenu
                {

                  // check the master collection for the artifact 
                  loadedComponent_ = findComponent(collectionName, "video", basename, "", false);

                  // check the collection for the artifact
                  if(!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->collectionInfo->name, "video", basename, "", false);
                  }

                  // check the submenu collection for the system artifact
                  if (!loadedComponent_)
                  {
                    loadedComponent_ = findComponent(selectedItem->name, "video", "video", "", true);
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

    // check for images, also if video could not be found (and was specified)
    for(unsigned int n = 0; n < names.size() && !loadedComponent_; ++n)
    {
        std::string basename = names[n];
        std::string typeLC   = Utils::toLower(type_);
        bool        defined  = false;

        if(basename == "default")
        {
            basename = "default";
            defined  = true;
        }
        else if(typeLC == "numberbuttons")
        {
            basename = selectedItem->numberButtons;
            defined  = true;
        }
        else if(typeLC == "numberplayers")
        {
            basename = selectedItem->numberPlayers;
            defined  = true;
        }
        else if(typeLC == "year")
        {
            basename = selectedItem->year;
            defined  = true;
        }
        else if(typeLC == "title")
        {
            basename = selectedItem->title;
            defined  = true;
        }
        else if(typeLC == "developer")
        {
            basename = selectedItem->developer;
            defined  = true;
            // Overwrite in case developer has not been specified
            if (basename == "")
            {
                basename = selectedItem->manufacturer;
            }
        }
        else if(typeLC == "manufacturer")
        {
            basename = selectedItem->manufacturer;
            defined  = true;
        }
        else if(typeLC == "genre")
        {
            basename = selectedItem->genre;
            defined  = true;
        }
        else if(typeLC == "ctrltype")
        {
            basename = selectedItem->ctrlType;
            defined  = true;
        }
        else if(typeLC == "joyways")
        {
            basename = selectedItem->joyWays;
            defined  = true;
        }
        else if(typeLC == "rating")
        {
            basename = selectedItem->rating;
            defined  = true;
        }
        else if(typeLC == "score")
        {
            basename = selectedItem->score;
            defined  = true;
        }
        else if(typeLC == "playlist")
        {
            basename = page.getPlaylistName();
            defined  = true;
        }
        else if (typeLC == "firstletter")
        {
            basename = selectedItem->fullTitle.at(0);
            defined  = true;
        }

        if (!selectedItem->leaf) // item is not a leaf
        {
            (void)config_.getProperty("collections." + selectedItem->name + "." + type_, basename );
        }

        bool overwriteXML = false;
        config_.getProperty( "overwriteXML", overwriteXML );
        if ( !defined || overwriteXML ) // No basename was found yet; check the info in stead
        {
            std::string basename_tmp;
            selectedItem->getInfo( type_, basename_tmp );
            if ( basename_tmp != "" )
            {
                basename = basename_tmp;
            }
        }

        Utils::replaceSlashesWithUnderscores(basename);

        if(systemMode_)
        {

            // check the master collection for the system artifact 
            loadedComponent_ = findComponent(collectionName, type_, type_, "", true);

            // check collection for the system artifact
            if(!loadedComponent_)
            {
              loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, "", true);
            }

        }
        else
        {

            // are we looking at a leaf or a submenu
            if (selectedItem->leaf) // item is a leaf
            {

              // check the master collection for the artifact 
              loadedComponent_ = findComponent(collectionName, type_, basename, "", false);

              // check the collection for the artifact
              if(!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, "", false);
              }

              // check the rom directory for the artifact
              if(!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, type_, selectedItem->filepath, false);
              }

            }
            else // item is a submenu
            {

              // check the master collection for the artifact 
              loadedComponent_ = findComponent(collectionName, type_, basename, "", false);

              // check the collection for the artifact
              if(!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->collectionInfo->name, type_, basename, "", false);
              }

              // check the submenu collection for the system artifact
              if (!loadedComponent_)
              {
                loadedComponent_ = findComponent(selectedItem->name, type_, type_, "", true);
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

    // if image and artwork was not specified, image fallback
    if(!loadedComponent_ && imageFallback_)
    {
        std::string imagePath;
        ImageBuilder imageBuild;
        //imagePath = Utils::combinePath(Configuration::isUserLayout_?Configuration::userPath:Configuration::absolutePath, "collections", collectionName );
        imagePath = Utils::combinePath(Configuration::absolutePath, "collections", collectionName ); // forcing absolutePath and folder "Collection" for backups
        imagePath = Utils::combinePath( imagePath, "system_artwork" );
        loadedComponent_ = imageBuild.CreateImage( imagePath, page, std::string("fallback"), scaleX_, scaleY_, ditheringAuthorized_ );
    }

    // if image and artwork was not specified, and no image fallback, fall back to displaying text
    if(!loadedComponent_ && textFallback_)
    {
        loadedComponent_ = new Text(selectedItem->fullTitle, page, FfntInst_, scaleX_, scaleY_);
        baseViewInfo.ImageWidth = loadedComponent_->baseViewInfo.ImageWidth;
        baseViewInfo.ImageHeight = loadedComponent_->baseViewInfo.ImageHeight;
    }
}


Component *ReloadableMedia::findComponent(std::string collection, std::string type, std::string basename, std::string filepath, bool systemMode)
{
    std::string imagePath;
    Component *component = NULL;
    VideoBuilder videoBuild;
    ImageBuilder imageBuild;

    // check the system folder
    if (layoutMode_)
    {
        std::string layoutName;
        config_.getProperty("layout", layoutName);
        if (commonMode_)
        {
            imagePath = Utils::combinePath(Configuration::isUserLayout_?Configuration::userPath:Configuration::absolutePath, "layouts", layoutName, "collections", "_common");
        }
        else
        {
            imagePath = Utils::combinePath(Configuration::isUserLayout_?Configuration::userPath:Configuration::absolutePath, "layouts", layoutName, "collections", collection);
        }
        if (systemMode)
            imagePath = Utils::combinePath(imagePath, "system_artwork");
        else
            imagePath = Utils::combinePath(imagePath, "medium_artwork", type);
    }
    else
    {
        if (commonMode_)
        {
            imagePath = Utils::combinePath(Configuration::isUserLayout_?Configuration::userPath:Configuration::absolutePath, "collections", "_common" );
            if (systemMode)
                imagePath = Utils::combinePath(imagePath, "system_artwork");
            else
                imagePath = Utils::combinePath(imagePath, "medium_artwork", type);
        }
        else
        {
            config_.getMediaPropertyAbsolutePath(collection, type, systemMode, imagePath);
        }
    }
    if ( filepath != "" )
        imagePath = filepath;

    if(type == "video")
    {
        component = videoBuild.createVideo(imagePath, page, basename, scaleX_, scaleY_);
    }
    else
    {
        component = imageBuild.CreateImage(imagePath, page, basename, scaleX_, scaleY_, ditheringAuthorized_);
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
