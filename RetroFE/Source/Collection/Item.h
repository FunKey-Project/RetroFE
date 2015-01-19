/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
    bool operator<(const Item& rhs);
    bool operator>(const Item& rhs);

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

