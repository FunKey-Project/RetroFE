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
#include <time.h>
#include <locale>

static bool ImportConfiguration(Configuration *c);
static bool StartLogging();

int main(int argc, char **argv)
{

    // check to see if version or help was requested
    if(argc > 1)
    {
        std::string program = argv[0];
        std::string param   = argv[1];

        if(argc == 3 && param == "-createcollection")
        {
            // Do nothing; we handle that later
        }
        else if(param == "-version"  ||
                param == "--version" ||
                param == "-v")
        {
            std::cout << "RetroFE version " << Version::getString( ) << std::endl;
            return 0;
        }
        else
        {
            std::cout << "Usage:" << std::endl;
            std::cout << program  << "                                           Run RetroFE"                              << std::endl;
            std::cout << program  << " --version                                 Print the version of RetroFE."            << std::endl;
            std::cout << program  << " -createcollection <collection name>       Create a collection directory structure." << std::endl;
            return 0;
        }
    }

    // Initialize locale language
    setlocale( LC_ALL, "" );

    // Initialize random seed
    srand(static_cast<unsigned int>(time(0)));

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
	// Exit with a heads up...
	std::string logFile = Utils::combinePath(Configuration::absolutePath, "log.txt");
        fprintf(stderr, "RetroFE has failed to start due to configuration error.\nCheck log for details: %s\n", logFile.c_str());
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

            std::string infoFile = Utils::combinePath(collectionsPath, collection, "info.conf");

            c->import(collection, prefix, infoFile, false);

            std::string settingsFile = Utils::combinePath(collectionsPath, collection, "settings.conf");

            if(!c->import(collection, prefix, settingsFile, false))
            {
                Logger::write(Logger::ZONE_INFO, "RetroFE", "Could not import \"" + settingsFile + "\"");
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
	// Can't write to logs give a heads up...
	fprintf(stderr, "Could not open log: %s for writing!\nRetroFE will now exit...\n", logFile.c_str());
        //Logger::write(Logger::ZONE_ERROR, "RetroFE", "Could not open \"" + logFile + "\" for writing");
        return false;
    }

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Version " + Version::getString() + " starting");

#ifdef WIN32
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Windows");
#elif __APPLE__
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Mac");
#else
    Logger::write(Logger::ZONE_INFO, "RetroFE", "OS: Linux");
#endif

    Logger::write(Logger::ZONE_INFO, "RetroFE", "Absolute path: " + Configuration::absolutePath);

    return true;
}
