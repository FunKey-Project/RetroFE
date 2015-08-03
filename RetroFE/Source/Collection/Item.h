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
#pragma once

#include <string>
#include "CollectionInfo.h"

class Item
{
public:
    Item();
    virtual ~Item();
    std::string filename();
    std::string lowercaseTitle() ;
    std::string lowercaseFullTitle();
    std::string name;
    std::string filepath;
    std::string title;
    std::string fullTitle;
    std::string year;
    std::string manufacturer;
    std::string genre;
    std::string cloneof;
    std::string numberPlayers;
    std::string numberButtons;
    CollectionInfo *collectionInfo;
    bool leaf;
};

