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
    Configuration::Initialize();

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
            CollectionInfoBuilder::CreateCollectionDirectory(value);
        }

        return 0;
    }


    if(!ImportConfiguration(&config))
    {
        return -1;
    }

    RetroFE p(config);

    p.Run();

    p.DeInitialize();

    Logger::DeInitialize();

    return 0;
}

bool ImportConfiguration(Configuration *c)
{
    std::string configPath =  Configuration::GetAbsolutePath();
    std::string launchersPath =  Utils::CombinePath(Configuration::GetAbsolutePath(), "launchers");
    std::string collectionsPath =  Utils::CombinePath(Configuration::GetAbsolutePath(), "collections");
    DIR *dp;
    struct dirent *dirp;

    std::string settingsConfPath = Utils::CombinePath(configPath, "settings.conf");
    if(!c->Import("", settingsConfPath))
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + settingsConfPath + "\"");
        return false;
    }
    
    std::string controlsConfPath = Utils::CombinePath(configPath, "controls.conf");
    if(!c->Import("controls", controlsConfPath))
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + controlsConfPath + "\"");
        return false;
    }

    dp = opendir(launchersPath.c_str());

    if(dp == NULL)
    {
        Logger::Write(Logger::ZONE_NOTICE, "RetroFE", "Could not read directory \"" + launchersPath + "\"");
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

                std::string importFile = Utils::CombinePath(launchersPath, std::string(dirp->d_name));

                if(!c->Import(prefix, importFile))
                {
                    Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + importFile + "\"");
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
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not read directory \"" + collectionsPath + "\"");
        return false;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        std::string dirName = (dirp->d_name);
        if (dirp->d_type == DT_DIR && dirName != "." && dirName != ".." && dirName.length() > 0 && dirName[0] != '_')
        {
            std::string prefix = "collections." + std::string(dirp->d_name);

            std::string settingsFile = Utils::CombinePath(collectionsPath, std::string(dirp->d_name), "settings.conf");

            if(!c->Import(prefix, settingsFile))
            {
                Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + settingsFile + "\"");
                closedir(dp);
                return false;
            }
        }
    }

    closedir(dp);

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Imported configuration");

    return true;
}

bool StartLogging()
{
    std::string logFile = Utils::CombinePath(Configuration::GetAbsolutePath(), "log.txt");

    if(!Logger::Initialize(logFile))
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not open \"" + logFile + "\" for writing");
        return false;
    }

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Version " + Version::GetString() + " starting");

#ifdef WIN32
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "OS: Windows");
#else
    Logger::Write(Logger::ZONE_INFO, "RetroFE", "OS: Linux");
#endif

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Absolute path: " + Configuration::GetAbsolutePath());

    return true;
}
