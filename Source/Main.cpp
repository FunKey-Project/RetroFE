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

#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

static bool ImportConfiguration(Configuration *c);

int main(int argc, char *argv[])
{
   Configuration config;
   const char *environment = std::getenv("RETROFE_PATH");
   std::string environmentStr;
   if (environment != NULL)
   {
      environmentStr = environment;
      Configuration::SetAbsolutePath(environment);
   }
   else
   {
#ifdef WIN32
      HMODULE hModule = GetModuleHandle(NULL);
      CHAR exe[MAX_PATH];
      GetModuleFileName(hModule, exe, MAX_PATH);
      std::string sPath(exe);
      sPath = Utils::GetDirectory(sPath);
      sPath = Utils::GetParentDirectory(sPath);
#else
      char exepath[1024];
      sprintf(exepath, "/proc/%d/exe", getpid());
      readlink(exepath, exepath, sizeof(exepath));
      std::string sPath(exepath);
      sPath = Utils::GetDirectory(sPath);
#endif


      Configuration::SetAbsolutePath(sPath);
   }

   // set the log file to write to
   std::string logFile = Configuration::GetAbsolutePath() + "/Log.txt";

   if(!Logger::StartLogFile(logFile))
   {
      Logger::Write(Logger::ZONE_ERROR, "RetroFE", "Could not open \"" + logFile + "\" for writing");
   }

   Logger::Write(Logger::ZONE_INFO, "RetroFE", "Version " + Version::GetString() + " starting");
#ifdef WIN32
   Logger::Write(Logger::ZONE_INFO, "RetroFE", "OS: Windows");
#else
   Logger::Write(Logger::ZONE_INFO, "RetroFE", "OS: Linux");
#endif

   if(environment)
   {
      Logger::Write(Logger::ZONE_INFO, "RetroFE", "Environment variable set: RetroFE_PATH=" + environmentStr);
   }

   Logger::Write(Logger::ZONE_INFO, "RetroFE", "Absolute path: " + Configuration::GetAbsolutePath());

   if(!ImportConfiguration(&config))
   {
	   return -1;
   }

   Logger::Write(Logger::ZONE_INFO, "RetroFE", "Imported configuration");

   std::string dbFile = (Configuration::GetAbsolutePath() + "/cache.db");
   std::ifstream infile(dbFile.c_str());
   DB db;
   if(!db.Initialize())
   {
	   return -1;
   }

   CollectionDatabase cdb(&db, &config);

   cdb.CheckDatabase();

   if(cdb.Import())
   {
      RetroFE p(&cdb, &config);

      if(p.Initialize())
      {
         p.Run();
      }

      p.DeInitialize();
   }

   Logger::CloseLogFile();
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

   return true;
}
