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
#include "CollectionInfoBuilder.h"
#include "CollectionInfo.h"
#include "../Database/Configuration.h"
#include "../Utility/Log.h"
#include <sstream>
#include <vector>

CollectionInfoBuilder::CollectionInfoBuilder(Configuration &c)
    : Conf(c)
{
}

CollectionInfoBuilder::~CollectionInfoBuilder()
{
    std::map<std::string, CollectionInfo *>::iterator it = InfoMap.begin();

    for(it == InfoMap.begin(); it != InfoMap.end(); ++it)
    {
        delete it->second;
    }

    InfoMap.clear();
}

bool CollectionInfoBuilder::LoadAllCollections()
{
    std::vector<std::string> collections;

    Conf.GetChildKeyCrumbs("collections", collections);

    if(collections.size() == 0)
    {
        Logger::Write(Logger::ZONE_ERROR, "Collections", "No collections were found. Please configure Settings.conf");
        return false;
    }

    bool retVal = true;
    std::vector<std::string>::iterator it;

    for(it = collections.begin(); it != collections.end(); ++it)
    {
        // todo: There is nothing that should really stop us from creating a collection
        //       in the main folder. I just need to find some time to look at the impacts if
        //       I remove this conditional check.
        if(*it != "Main")
        {
            if(ImportCollection(*it))
            {
                Logger::Write(Logger::ZONE_INFO, "Collections", "Adding collection " + *it);
            }
            else
            {
                // Continue processing the rest of the collections if an error occurs during import.
                // ImportCollection() will print out an error to the log file.
                retVal = false;
            }
        }
    }

    return retVal;
}

void CollectionInfoBuilder::GetCollections(std::vector<CollectionInfo *> &collections)
{
    std::map<std::string, CollectionInfo *>::iterator InfoMapIt;

    for(InfoMapIt = InfoMap.begin(); InfoMapIt != InfoMap.end(); ++InfoMapIt)
    {
        collections.push_back(InfoMapIt->second);
    }
}

bool CollectionInfoBuilder::ImportCollection(std::string name)
{
    // create a new instance if one does not exist
    if(InfoMap.find(name) != InfoMap.end())
    {
        return true;
    }
    std::string listItemsPathKey = "collections." + name + ".list.path";
    std::string listFilterKey = "collections." + name + ".list.filter";
    std::string extensionsKey = "collections." + name + ".list.extensions";
    std::string launcherKey = "collections." + name + ".launcher";

    //todo: metadata is not fully not implemented
    std::string metadataTypeKey = "collections." + name + ".metadata.type";
    std::string metadataPathKey = "collections." + name + ".metadata.path";

    std::string listItemsPath;
    std::string launcherName;
    std::string extensions;
    std::string metadataType;
    std::string metadataPath;

    if(!Conf.GetPropertyAbsolutePath(listItemsPathKey, listItemsPath))
    {
        Logger::Write(Logger::ZONE_INFO, "Collections", "Property \"" + listItemsPathKey + "\" does not exist. Assuming \"" + name + "\" is a menu");
        return false;
    }

    if(!Conf.GetProperty(extensionsKey, extensions))
    {
        Logger::Write(Logger::ZONE_INFO, "Collections", "Property \"" + extensionsKey + "\" does not exist. Assuming \"" + name + "\" is a menu");
        return false;
    }

    (void)Conf.GetProperty(metadataTypeKey, metadataType);
    (void)Conf.GetProperty(metadataPathKey, metadataPath);

    if(!Conf.GetProperty(launcherKey, launcherName))
    {
        std::stringstream ss;
        ss        << "Warning: launcher property  \""
                  << launcherKey
                  << "\" points to a launcher that is not configured (launchers."
                  << launcherName
                  << "). Your collection will be viewable, however you will not be able to "
                  << "launch any of the items in your collection.";

        Logger::Write(Logger::ZONE_WARNING, "Collections", ss.str());
    }

    InfoMap[name] = new CollectionInfo(name, listItemsPath, extensions, metadataType, metadataPath);

    return (InfoMap[name] != NULL);
}
