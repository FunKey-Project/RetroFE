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
#include "Item.h"
#include "../Database/Configuration.h"
#include "../Database/MetadataDatabase.h"
#include "../Database/DB.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include <dirent.h>
#include <sstream>
#include <vector>
#include <algorithm>

CollectionInfoBuilder::CollectionInfoBuilder(Configuration &c, MetadataDatabase &mdb)
    : Conf(c)
    , MetaDB(mdb)
{
}

CollectionInfoBuilder::~CollectionInfoBuilder()
{
}

CollectionInfo *CollectionInfoBuilder::BuildCollection(std::string name)
{
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
    std::string metadataType = name;
    std::string metadataPath;

    Conf.GetCollectionAbsolutePath(name, listItemsPath);
    (void)Conf.GetProperty(extensionsKey, extensions);
    (void)Conf.GetProperty(metadataTypeKey, metadataType);
    (void)Conf.GetProperty(metadataPathKey, metadataPath);

    if(!Conf.GetProperty(launcherKey, launcherName))
    {
        std::stringstream ss;
        ss        << "\""
                  << launcherKey
                  << "\" points to a launcher that is not configured (launchers."
                  << launcherName
                  << "). Your collection will be viewable, however you will not be able to "
                  << "launch any of the items in your collection.";

        Logger::Write(Logger::ZONE_NOTICE, "Collections", ss.str());
    }

    CollectionInfo *collection = new CollectionInfo(name, listItemsPath, extensions, metadataType, metadataPath);

    ImportDirectory(collection);

    return collection;
}


bool CollectionInfoBuilder::ImportBasicList(CollectionInfo * /*info*/, std::string file, std::map<std::string, Item *> &list)
{
    std::ifstream includeStream(file.c_str());

    if (!includeStream.good())
    {
        return false;
    }

    std::string line;

    while(std::getline(includeStream, line))
    {

        if(list.find(line) == list.end())
        {
            Item *i = new Item();

            line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );

            i->SetFullTitle(line);

            list[line] = i;
        }
    }

    return true;
}

bool CollectionInfoBuilder::ImportDirectory(CollectionInfo *info)
{
    DIR *dp;
    struct dirent *dirp;
    std::string path = info->GetListPath();
    std::map<std::string, Item *> includeFilter;
    std::map<std::string, Item *> excludeFilter;
    std::string includeFile = Configuration::GetAbsolutePath() + "/collections/" + info->GetName() + "/include.txt";
    std::string excludeFile = Configuration::GetAbsolutePath() + "/collections/" + info->GetName() + "/exclude.txt";
    std::string launcher;


    ImportBasicList(info, includeFile, includeFilter);
    ImportBasicList(info, excludeFile, excludeFilter);

    std::vector<std::string> extensions;
    std::vector<std::string>::iterator extensionsIt;

    info->GetExtensions(extensions);

    (void)Conf.GetProperty("collections." + info->GetName() + ".launcher", launcher);
    Logger::Write(Logger::ZONE_INFO, "CollectionInfoBuilder", "Checking for \"" + includeFile + "\"");

    dp = opendir(path.c_str());

    if(dp == NULL)
    {
        Logger::Write(Logger::ZONE_INFO, "CollectionInfoBuilder", "Could not read directory \"" + path + "\". Ignore if this is a menu.");
        return false;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        std::string file = dirp->d_name;

        Utils::NormalizeBackSlashes(file);
        size_t position = file.find_last_of(".");
        std::string basename = (std::string::npos == position)? file : file.substr(0, position);

        if((includeFilter.size() == 0 || includeFilter.find(basename) != includeFilter.end()) &&
                (excludeFilter.size() == 0 || excludeFilter.find(basename) == excludeFilter.end()))
        {
            for(extensionsIt = extensions.begin(); extensionsIt != extensions.end(); ++extensionsIt)
            {
                std::string comparator = "." + *extensionsIt;
                int start = file.length() - comparator.length() + 1;

                if(start >= 0)
                {
                    if(file.compare(start, comparator.length(), *extensionsIt) == 0)
                    {
                        Item *i = new Item();
                        i->SetName(basename);
                        i->SetFullTitle(basename);
                        i->SetTitle(basename);
                        i->SetLauncher(launcher);
                        info->GetItems()->push_back(i);
                    }
                }
            }
        }
    }

    closedir(dp);

    info->SortItems();

    MetaDB.InjectMetadata(info);

    while(includeFilter.size() > 0)
    {
        std::map<std::string, Item *>::iterator it = includeFilter.begin();
        delete it->second;
        includeFilter.erase(it);
    }
    while(excludeFilter.size() > 0)
    {
        std::map<std::string, Item *>::iterator it = excludeFilter.begin();
        delete it->second;
        excludeFilter.erase(it);
    }

    return true;
}
