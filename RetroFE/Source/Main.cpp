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

#include "Database/Configuration.h"
#include "Collection/CollectionInfoBuilder.h"
#include "Execute/Launcher.h"
#include "Utility/Log.h"
#include "Utility/Utils.h"
#include "RetroFE.h"
#include "Version.h"
#include <cstdlib>
#include <fstream>
#include <dirent.h>

static bool ImportConfiguration(Configuration *c);
static bool StartLogging();

int main(int argc, char **argv)
{
    Configuration::initialize();

    Configuration config;

    if(!StartLogging())
    {
        return -1;
    }

    // check to see if createcollection was requested
    if(argc == 3)
    {
        std::string param = argv[1];
        std::string value = argv[2];

        if(param == "-createcollection")
        {
            CollectionInfoBuilder::createCollectionDirectory(value);
        }

        return 0;
    }


    if(!ImportConfiguration(&config))
    {
        return -1;
    }

    RetroFE p(config);

    p.run();

    p.deInitialize();

    Logger::deInitialize();

    return 0;
}

bool ImportConfiguration(Configuration *c)
{
    std::string configPath =  Configuration::absolutePath;
    std::string launchersPath =  Utils::combinePath(Configuration::absolutePath, "launchers");
    std::string collectionsPath =  Utils::combinePath(Configuration::absolutePath, "collections");
    DIR *dp;
    struct dirent *dirp;

    std::string settingsConfPath = Utils::combinePath(configPath, "settings.conf");
    if(!c->import("", settingsConfPath))
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + settingsConfPath + "\"");
        return false;
    }
    
    std::string controlsConfPath = Utils::combinePath(configPath, "controls.conf");
    if(!c->import("controls", controlsConfPath))
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + controlsConfPath + "\"");
        return false;
    }

    dp = opendir(launchersPath.c_str());

    if(dp == NULL)
    {
        Logger::write(Logger::ZONE_NOTICE, "RetroFE", "Could not read directory \"" + launchersPath + "\"");
        return false;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type != DT_DIR && std::string(dirp->d_name) != "." && std::string(dirp->d_name) != "..")
        {
            std::string basename = dirp->d_name;

            std::string extension = basename.substr(basename.find_last_of("."), basename.size()-1);
            basename = basename.substr(0, basename.find_last_of("."));

            if(extension == ".conf")
            {
                std::string prefix = "launchers." + basename;

                std::string importFile = Utils::combinePath(launchersPath, std::string(dirp->d_name));

                if(!c->import(prefix, importFile))
                {
                    Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + importFile + "\"");
                    closedir(dp);
                    return false;
                }
            }
        }
    }

    closedir(dp);

    dp = opendir(collectionsPath.c_str());

    if(dp == NULL)
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not read directory \"" + collectionsPath + "\"");
        return false;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        std::string collection = (dirp->d_name);
        if (dirp->d_type == DT_DIR && collection != "." && collection != ".." && collection.length() > 0 && collection[0] != '_')
        {
            std::string prefix = "collections." + collection;

            std::string settingsFile = Utils::combinePath(collectionsPath, collection, "settings.conf");

            if(!c->import(collection, prefix, settingsFile))
            {
                Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + settingsFile + "\"");
                closedir(dp);
                return false;
            }
        }
    }

    closedir(dp);

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Imported configuration");

    return true;
}

bool StartLogging()
{
    std::string logFile = Utils::combinePath(Configuration::absolutePath, "log.txt");

    if(!Logger::initialize(logFile))
    {
        Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not open \"" + logFile + "\" for writing");
        return false;
    }

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Version " + Version::getString() + " starting");

#ifdef WIN32
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Windows");
#else
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Linux");
#endif

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Absolute path: " + Configuration::absolutePath);

    return true;
}
