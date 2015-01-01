/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Database/Configuration.h"
#include "Database/CollectionDatabase.h"
#include "Collection/CollectionInfoBuilder.h"
#include "Collection/CollectionInfo.h"
#include "Database/DB.h"
#include "Database/MamelistMetadata.h"
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
CollectionDatabase *InitializeCollectionDatabase(DB &db, Configuration &config);

int main(int argc, char *argv[])
{
    Configuration::Initialize();

    Configuration config;

    if(!StartLogging())
    {
        return -1;
    }

    if(!ImportConfiguration(&config))
    {
        return -1;
    }

    DB db(Configuration::GetAbsolutePath() + "/cache.db");

    if(!db.Initialize())
    {
        return -1;
    }


    CollectionDatabase *cdb = InitializeCollectionDatabase(db, config);
    if(!cdb)
    {
        return -1;
    }

    RetroFE p(cdb, &config);

    if(p.Initialize())
    {
        p.Run();
    }

    p.DeInitialize();

    Logger::DeInitialize();

    return 0;
}

bool ImportConfiguration(Configuration *c)
{
    std::string configPath =  Configuration::GetAbsolutePath();
    std::string launchersPath =  Configuration::GetAbsolutePath() + "/Launchers";
    std::string collectionsPath =  Configuration::GetAbsolutePath() + "/Collections";
    DIR *dp;
    struct dirent *dirp;

    if(!c->Import("", configPath + "/Settings.conf"))
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + configPath + "/Settings.conf\"");
        return false;
    }

    if(!c->Import("controls", configPath + "/Controls.conf"))
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + configPath + "/Settings.conf\"");
        return false;
    }

    dp = opendir(launchersPath.c_str());

    if(dp == NULL)
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not read directory \"" + launchersPath + "\"");
        return false;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type != DT_DIR && std::string(dirp->d_name) != "." && std::string(dirp->d_name) != "..")
        {
            std::string basename = dirp->d_name;

            //  if(basename.length() > 0)
            {
                std::string extension = basename.substr(basename.find_last_of("."), basename.size()-1);
                basename = basename.substr(0, basename.find_last_of("."));

                if(extension == ".conf")
                {
                    std::string prefix = "launchers." + basename;

                    std::string importFile = launchersPath + "/" + std::string(dirp->d_name);

                    if(!c->Import(prefix, importFile))
                    {
                        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + importFile + "\"");
                        return false;
                    }
                }
            }
        }
    }

    dp = opendir(collectionsPath.c_str());

    if(dp == NULL)
    {
        Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not read directory \"" + collectionsPath + "\"");
        return false;
    }

    while((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type == DT_DIR && std::string(dirp->d_name) != "." && std::string(dirp->d_name) != "..")
        {
            std::string prefix = "collections." + std::string(dirp->d_name);

            std::string settingsFile = collectionsPath + "/" + dirp->d_name + "/Settings.conf";

            if(!c->Import(prefix, settingsFile))
            {
                Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not import \"" + settingsFile + "\"");
                return false;
            }
        }
    }

    Logger::Write(Logger::ZONE_INFO, "RetroFE", "Imported configuration");

    return true;
}

bool StartLogging()
{
    std::string logFile = Configuration::GetAbsolutePath() + "/Log.txt";

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

CollectionDatabase *InitializeCollectionDatabase(DB &db, Configuration &config)
{
    CollectionDatabase *cdb = NULL;
    std::string dbFile = (Configuration::GetAbsolutePath() + "/cache.db");
    std::ifstream infile(dbFile.c_str());

    cdb = new CollectionDatabase(&db, &config);

    if(!cdb->Initialize())
    {
        delete cdb;
        cdb = NULL;
    }
    else if(!cdb->Import())
    {
        delete cdb;
        cdb = NULL;
    }

    return cdb;
}