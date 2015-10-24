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

#include "ReloadableText.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../SDL.h"
#include <fstream>
#include <vector>
#include <iostream>

ReloadableText::ReloadableText(std::string type, Page &page, Font *font, std::string layoutKey, float scaleX, float scaleY)
    : Component(page)
    , imageInst_(NULL)
    , layoutKey_(layoutKey)
    , reloadRequested_(false)
    , firstLoad_(true)
    , fontInst_(font)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
{

    type_ = TextTypeUnknown;

    if(type == "numberButtons")
    {
        type_ = TextTypeNumberButtons;
    }
    else if(type == "numberPlayers")
    {
        type_ = TextTypeNumberPlayers;
    }
    else if(type == "year")
    {
        type_ = TextTypeYear;
    }
    else if(type == "title")
    {
        type_ = TextTypeTitle;
    }
    else if(type == "manufacturer")
    {
        type_ = TextTypeManufacturer;
    }
    else if(type == "genre")
    {
        type_ = TextTypeGenre;
    }
    else if(type == "playlist")
    {
        type_ = TextTypePlaylist;
    }
    else if(type == "collectionName")
    {
        type_ = TextTypeCollectionName;
    }
    else if(type == "collectionSize")
    {
        type_ = TextTypeCollectionSize;
    }
    else if(type == "collectionIndex")
    {
        type_ = TextTypeCollectionIndex;
    }
    allocateGraphicsMemory();
}



ReloadableText::~ReloadableText()
{
    if (imageInst_ != NULL)
    {
        delete imageInst_;
    }
}

void ReloadableText::update(float dt)
{
    if((type_ != TextTypePlaylist && newItemSelected) || 
       (type_ == TextTypePlaylist && playlistChanged))
    {
        reloadRequested_ = true;
    }
    // wait for the right moment to reload the image
    if (reloadRequested_ && (highlightExitComplete || firstLoad_))
    {
        ReloadTexture();
        reloadRequested_ = false;
        firstLoad_ = false;
    }

    // needs to be ran at the end to prevent the NewItemSelected flag from being detected
    Component::update(dt);

}

void ReloadableText::allocateGraphicsMemory()
{
    firstLoad_ = true;

    ReloadTexture();

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::allocateGraphicsMemory();
}

void ReloadableText::launchEnter()
{
}

void ReloadableText::launchExit()
{
}

void ReloadableText::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();

    if (imageInst_ != NULL)
    {
        delete imageInst_;
        imageInst_ = NULL;
    }
}
void ReloadableText::ReloadTexture()
{
    if (imageInst_ != NULL)
    {
        delete imageInst_;
        imageInst_ = NULL;
    }

    Item *selectedItem = page.getSelectedItem();

    if (selectedItem != NULL)
    {
        std::stringstream ss;
        std::string text;
        switch(type_)
        {
        case TextTypeNumberButtons:
            ss << selectedItem->numberButtons;
            break;
        case TextTypeNumberPlayers:
            ss << selectedItem->numberPlayers;
            break;
        case TextTypeYear:
            ss << selectedItem->year;
            break;
        case TextTypeTitle:
            ss << selectedItem->title;
            break;
        case TextTypeManufacturer:
            ss << selectedItem->manufacturer;
            break;
        case TextTypeGenre:
            ss << selectedItem->genre;
            break;
        case TextTypePlaylist:
            ss << playlistName;
            break;
        case TextTypeCollectionName:
            ss << page.getCollectionName();
            break;
        case TextTypeCollectionSize:
            ss << page.getCollectionSize();
            break;
        case TextTypeCollectionIndex:
              ss << (1+page.getSelectedIndex());
            break;

        default:
            break;
        }

        imageInst_ = new Text(ss.str(), page, fontInst_, scaleX_, scaleY_);
    }
}


void ReloadableText::draw()
{
    if(imageInst_)
    {
        imageInst_->baseViewInfo = baseViewInfo;
        imageInst_->draw();
    }
}
