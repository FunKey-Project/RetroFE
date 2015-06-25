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
#include "CollectionInfo.h"
#include "Item.h"
#include "../Database/Configuration.h"
#include "../Utility/Utils.h"
#include <sstream>
#include <algorithm>

CollectionInfo::CollectionInfo(std::string name,
                               std::string listPath,
                               std::string extensions,
                               std::string metadataType,
                               std::string metadataPath)
    : name(name)
    , listpath(listPath)
    , metadataType(metadataType)
    , metadataPath_(metadataPath)
	, extensions_(extensions)
{
}

CollectionInfo::~CollectionInfo()
{
    std::vector<Item *>::iterator it = items.begin();

    while(it != items.end())
    {
        delete *it;
        items.erase(it);
        it = items.begin();
    }
}

std::string CollectionInfo::settingsPath() const
{
    return Utils::combinePath(Configuration::absolutePath, "collections", name);
}


void CollectionInfo::extensionList(std::vector<std::string> &extensionlist)
{
    std::istringstream ss(extensions_);
    std::string token;

    while(std::getline(ss, token, ','))
    {
    	extensionlist.push_back(token);
    }
}


bool CollectionInfo::itemIsLess(Item *lhs, Item *rhs)
{
    return lhs->lowercaseFullTitle() < rhs->lowercaseFullTitle();
}

void CollectionInfo::sortItems()
{
    std::sort(items.begin(), items.end(), itemIsLess);
}
