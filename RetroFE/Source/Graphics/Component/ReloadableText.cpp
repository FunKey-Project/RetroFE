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

ReloadableText::ReloadableText(std::string type, Font *font, SDL_Color color, std::string layoutKey, std::string collection, float scaleX, float scaleY)
    : ImageInst(NULL)
    , LayoutKey(layoutKey)
    , Collection(collection)
    , ReloadRequested(false)
    , FirstLoad(true)
    , FontInst(font)
    , FontColor(color)
    , ScaleX(scaleX)
    , ScaleY(scaleY)
{

    Type = TextTypeUnknown;

    if(type == "numberButtons")
    {
        Type = TextTypeNumberButtons;
    }
    else if(type == "numberPlayers")
    {
        Type = TextTypeNumberPlayers;
    }
    else if(type == "year")
    {
        Type = TextTypeYear;
    }
    else if(type == "title")
    {
        Type = TextTypeTitle;
    }
    else if(type == "manufacturer")
    {
        Type = TextTypeManufacturer;
    }

    AllocateGraphicsMemory();
}



ReloadableText::~ReloadableText()
{
    if (ImageInst != NULL)
    {
        delete ImageInst;
    }
}

void ReloadableText::Update(float dt)
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

    // needs to be ran at the end to prevent the NewItemSelected flag from being detected
    Component::Update(dt);

}

void ReloadableText::AllocateGraphicsMemory()
{
    FirstLoad = true;

    ReloadTexture();

    // NOTICE! needs to be done last to prevent flags from being missed
    Component::AllocateGraphicsMemory();
}

void ReloadableText::LaunchEnter()
{
}

void ReloadableText::LaunchExit()
{
}

void ReloadableText::FreeGraphicsMemory()
{
    Component::FreeGraphicsMemory();

    if (ImageInst != NULL)
    {
        delete ImageInst;
        ImageInst = NULL;
    }
}
void ReloadableText::ReloadTexture()
{
    if (ImageInst != NULL)
    {
        delete ImageInst;
        ImageInst = NULL;
    }

    Item *selectedItem = GetSelectedItem();

    if (selectedItem != NULL)
    {
        std::stringstream ss;
        std::string text;
        switch(Type)
        {
        case TextTypeNumberButtons:
            ss << selectedItem->GetNumberButtons();
            break;
        case TextTypeNumberPlayers:
            ss << selectedItem->GetNumberPlayers();
            break;
        case TextTypeYear:
            ss << selectedItem->GetYear();
            break;
        case TextTypeTitle:
            ss << selectedItem->GetTitle();
            break;
        case TextTypeManufacturer:
            ss << selectedItem->GetManufacturer();
            break;
        default:
            break;
        }

        ImageInst = new Text(ss.str(), FontInst, FontColor, ScaleX, ScaleY);
    }
}


void ReloadableText::Draw()
{
    ViewInfo *info = GetBaseViewInfo();

    if(ImageInst)
    {
        ImageInst->UpdateBaseViewInfo(*info);
        ImageInst->Draw();
    }
}
