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

class Item
{
public:
    Item();
    virtual ~Item();
    std::string FileName();
    std::string LCTitle() ;
    std::string LCFullTitle();
    std::string Name;
    std::string Launcher;
    std::string FilePath;
    std::string Title;
    std::string FullTitle;
    std::string Year;
    std::string Manufacturer;
    std::string Genre;
    std::string CloneOf;
    std::string NumberPlayers;
    std::string NumberButtons;
    bool Leaf;
};

