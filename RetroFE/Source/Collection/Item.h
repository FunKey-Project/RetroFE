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
    const std::string GetFileName() const;
    const std::string& GetFilePath() const;
    void SetFilePath(const std::string& filepath);
    const std::string& GetLauncher() const;
    void SetLauncher(const std::string& launcher);
    const std::string& GetManufacturer() const;
    void SetManufacturer(const std::string& manufacturer);
    const std::string& GetName() const;
    void SetName(const std::string& name);
    void SetNumberButtons(std::string numberbuttons);
    std::string GetNumberButtons() const;
    void SetNumberPlayers(std::string numberplayers);
    std::string GetNumberPlayers() const;
    const std::string& GetTitle() const;
    const std::string& GetLCTitle() const;
    void SetTitle(const std::string& title);
    const std::string& GetYear() const;
    void SetYear(const std::string& year);
    bool IsLeaf() const;
    void SetIsLeaf(bool leaf);
    const std::string& GetFullTitle() const;
    void SetFullTitle(const std::string& fulltitle);
    const std::string& GetCloneOf() const;
    void SetCloneOf(const std::string& cloneOf);

private:
    std::string Launcher;
    std::string FilePath;
    std::string Name;
    std::string Title;
    std::string LCTitle;
    std::string FullTitle;
    std::string Year;
    std::string Manufacturer;
    std::string CloneOf;
    std::string NumberPlayers;
    std::string NumberButtons;
    bool Leaf;
};

