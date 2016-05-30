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

ReloadableText::ReloadableText(std::string type, Page &page, Configuration &config, Font *font, std::string layoutKey, float scaleX, float scaleY)
    : Component(page)
    , config_(config)
    , imageInst_(NULL)
    , type_(type)
    , layoutKey_(layoutKey)
    , reloadRequested_(false)
    , firstLoad_(true)
    , fontInst_(font)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
{
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
    if((type_ != "playlist" && newItemSelected) || 
       (type_ == "playlist" && playlistChanged))
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
        if (type_ == "numberButtons")
        {
            ss << selectedItem->numberButtons;
        }
        else if (type_ == "numberPlayers")
        {
            ss << selectedItem->numberPlayers;
        }
        else if (type_ == "numberPlayersRange")
        {
          if (selectedItem->numberPlayers != ""  &&
              selectedItem->numberPlayers != "0" &&
              selectedItem->numberPlayers != "1")
            ss << "1-" << selectedItem->numberPlayers;
          else
            ss << selectedItem->numberPlayers;
        }
        else if (type_ == "numberJoyWays")
        {
            ss << selectedItem->joyWays;
        }
        else if (type_ == "rating")
        {
            ss << selectedItem->rating;
        }
        else if (type_ == "year")
        {
            if (selectedItem->leaf) // item is a leaf
              text = selectedItem->year;
            else // item is a collection
              (void)config_.getProperty("collections." + selectedItem->name + ".year", text );
            ss << text;
        }
        else if (type_ == "title")
        {
            ss << selectedItem->title;
        }
        else if (type_ == "manufacturer")
        {
            if (selectedItem->leaf) // item is a leaf
              text = selectedItem->manufacturer;
            else // item is a collection
              (void)config_.getProperty("collections." + selectedItem->name + ".manufacturer", text );
            ss << text;
        }
        else if (type_ == "genre")
        {
            if (selectedItem->leaf) // item is a leaf
              text = selectedItem->genre;
            else // item is a collection
              (void)config_.getProperty("collections." + selectedItem->name + ".genre", text );
            ss << text;
        }
        else if (type_ == "playlist")
        {
            ss << playlistName;
        }
        else if (type_ == "collectionName")
        {
            ss << page.getCollectionName();
        }
        else if (type_ == "collectionSize")
        {
            ss << page.getCollectionSize();
        }
        else if (type_ == "collectionIndex")
        {
              ss << (1+page.getSelectedIndex());
        } else if (!selectedItem->leaf) // item is not a leaf
        {
            (void)config_.getProperty("collections." + selectedItem->name + "." + type_, text );
            ss << text;
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
