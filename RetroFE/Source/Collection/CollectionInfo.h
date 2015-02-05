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
#include <vector>

class Item;

class CollectionInfo
{
public:
    CollectionInfo(std::string name, std::string listPath, std::string extensions, std::string metadataType, std::string metadataPath);
    virtual ~CollectionInfo();
    std::string GetName() const;
    std::string GetSettingsPath() const;
    std::string GetListPath() const;
    std::string GetMetadataType() const;
    std::string GetMetadataPath() const;
    std::string GetExtensions() const;
    std::vector<Item *> *GetItems();
    void SortItems();
    void GetExtensions(std::vector<std::string> &extensions);

private:
    static bool ItemIsLess(Item const *lhs, Item const *rhs);  

    std::string Name;
    std::string ListPath;
    std::string Extensions;
    std::string MetadataType;
    std::string MetadataPath;
    std::vector<Item *> Items;
};
