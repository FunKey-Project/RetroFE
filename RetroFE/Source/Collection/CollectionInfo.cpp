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
#include "../Utility/Log.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <exception>

#ifdef __linux
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#endif

CollectionInfo::CollectionInfo(std::string name,
                               std::string listPath,
                               std::string extensions,
                               std::string metadataType,
                               std::string metadataPath)
    : name(name)
    , listpath(listPath)
    , saveRequest(false)
    , metadataType(metadataType)
    , menusort(true)
    , metadataPath_(metadataPath)
	, extensions_(extensions)
{
}

CollectionInfo::~CollectionInfo()
{
	// remove items from the subcollections so their destructors do not
	// delete the items since the parent collection will delete them.
    std::vector<CollectionInfo *>::iterator subit;
    for (subit = subcollections_.begin(); subit != subcollections_.end(); subit++)
    {
    	CollectionInfo *info = *subit;
    	info->items.clear();
    }


    Playlists_T::iterator pit = playlists.begin();

    while(pit != playlists.end())
    {
        if(pit->second != &items)
        {
            delete pit->second;
        }
        playlists.erase(pit);
        pit = playlists.begin();
    }

	std::vector<Item *>::iterator it = items.begin();
    while(it != items.end())
    {
        delete *it;
        items.erase(it);
        it = items.begin();
    }
}

bool CollectionInfo::Save() 
{
    bool retval = true;
    if(saveRequest)
    {
        std::string dir  = Utils::combinePath(Configuration::absolutePath, "collections", name, "playlists");
        std::string file = Utils::combinePath(Configuration::absolutePath, "collections", name, "playlists/favorites.txt");
        Logger::write(Logger::ZONE_INFO, "Collection", "Saving " + file);

        std::ofstream filestream;
        try
        {
            // Create playlists directory if it does not exist yet.
            struct stat info;
            if ( stat( dir.c_str(), &info ) != 0 && (info.st_mode & S_IFDIR) )
            {
#if defined(_WIN32) && !defined(__GNUC__)
                if(!CreateDirectory(dir, NULL))
                {
                    if(ERROR_ALREADY_EXISTS != GetLastError())
                    {
                        std::cout << "Could not create folder \"" << *it << "\"" << std::endl;
                        return false;
                    }
                }
#else 
#if defined(__MINGW32__)
                if(mkdir(dir.c_str()) == -1)
#else
                if(mkdir(dir.c_str(), 0755) == -1)
#endif        
                {
                   std::cout << "Could not create folder \"" << dir << "\":" << errno << std::endl;
                }
#endif
            }

            filestream.open(file.c_str());
            std::vector<Item *> *saveitems = playlists["favorites"];
            for(std::vector<Item *>::iterator it = saveitems->begin(); it != saveitems->end(); it++)
            {
                filestream << (*it)->name << std::endl;
            }

            filestream.close();
        }
        catch(std::exception &)
        {
            Logger::write(Logger::ZONE_ERROR, "Collection", "Save failed: " + file);
            retval = false;
        }
    }
    
    return retval;
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
    for(Playlists_T::iterator it = playlists.begin(); it != playlists.end(); it++)
    {
        std::sort(it->second->begin(), it->second->end(), itemIsLess);
    }
}
