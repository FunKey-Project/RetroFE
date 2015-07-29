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
    , menusort(true)
{
}

CollectionInfo::~CollectionInfo()
{
	// remove items from the subcollections so their destructors do not
	// delete the items since the parent collection will delete them.
    std::vector<CollectionInfo *>::iterator subit;
    for (subit != subcollections_.begin(); subit != subcollections_.end(); subit++)
    {
    	CollectionInfo *info = *subit;
    	info->items.clear();
    }


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

void CollectionInfo::addSubcollection(CollectionInfo *newinfo)
{
	subcollections_.push_back(newinfo);

    items.insert(items.begin(), newinfo->items.begin(), newinfo->items.end());
}

bool CollectionInfo::hasSubcollections()
{
    return (subcollections_.size() > 0);
}

bool CollectionInfo::itemIsLess(Item *lhs, Item *rhs)
{
    if(lhs->leaf && !rhs->leaf) return true;
    if(!lhs->leaf && rhs->leaf) return false;
    if(!lhs->collectionInfo->menusort && lhs->leaf && rhs->leaf) return false;
    return lhs->lowercaseFullTitle() < rhs->lowercaseFullTitle();
}


void CollectionInfo::sortItems()
{
    std::sort(items.begin(), items.end(), itemIsLess);
}
