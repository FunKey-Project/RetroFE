/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "CollectionDatabase.h"
#include "../Collection/CollectionInfoBuilder.h"
#include "../Collection/CollectionInfo.h"
#include "../Collection/Item.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include "MamelistMetadata.h"
#include "Configuration.h"
#include "DB.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <list>
#include <rapidxml.hpp>
#include <sstream>
#include <string>
#include <map>
#include <sys/types.h>
#include <sqlite3.h>
#include <zlib.h>
#include <exception>

CollectionDatabase::CollectionDatabase(DB *db, Configuration *c)
: Config(c)
, DBInstance(db)
{

}

CollectionDatabase::~CollectionDatabase()
{
}

bool CollectionDatabase::ResetDatabase()
{
   bool retVal = true;
   int rc;
   char *error = NULL;
   sqlite3 *handle = DBInstance->GetHandle();

   Logger::Write(Logger::ZONE_INFO, "Database", "Erasing");

   std::string sql;
   sql.append("DROP TABLE IF EXISTS CollectionItems;");
   sql.append("DROP TABLE IF EXISTS Meta;");
   sql.append("DROP TABLE IF EXISTS Collections;");

   rc = sqlite3_exec(handle, sql.c_str(), NULL, 0, &error);

   if(rc != SQLITE_OK)
   {
      std::stringstream ss;
      ss << "Unable to create Configurations table. Error: " << error;
      Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());
      retVal = false;
   }

  //CheckDatabase();

   return retVal;
}

bool CollectionDatabase::CheckDatabase()
{
   bool retVal = true;
   int rc;
   char *error = NULL;
   sqlite3 *handle = DBInstance->GetHandle();

   std::string sql;
   sql.append("CREATE TABLE IF NOT EXISTS CollectionItems(");
   sql.append("collectionName TEXT KEY,");
   sql.append("filePath TEXT NOT NULL DEFAULT '',");
   sql.append("name TEXT NOT NULL DEFAULT '',");
   sql.append("hidden INT NOT NULL DEFAULT 0);");

   sql.append("CREATE TABLE IF NOT EXISTS Meta(");
   sql.append("collectionName TEXT KEY,");
   sql.append("name TEXT NOT NULL DEFAULT '',");
   sql.append("title TEXT NOT NULL DEFAULT '',");
   sql.append("year TEXT NOT NULL DEFAULT '',");
   sql.append("manufacturer TEXT NOT NULL DEFAULT '',");
   sql.append("cloneOf TEXT NOT NULL DEFAULT '',");
   sql.append("players INTEGER,");
   sql.append("buttons INTEGER);");
   sql.append("CREATE UNIQUE INDEX IF NOT EXISTS MetaUniqueId ON Meta(collectionName, name);");

   sql.append("CREATE TABLE IF NOT EXISTS Collections(");
   sql.append("collectionName TEXT KEY,");
   sql.append("crc32 UNSIGNED INTEGER NOT NULL DEFAULT 0);");
   sql.append("CREATE UNIQUE INDEX IF NOT EXISTS CollectionsUniqueId ON Collections(collectionName);");

   rc = sqlite3_exec(handle, sql.c_str(), NULL, 0, &error);

   if(rc != SQLITE_OK)
   {
      std::stringstream ss;
      ss << "Unable to create Configurations table. Error: " << error;
      Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());

      retVal = false;
   }

   return retVal;
}


bool CollectionDatabase::Import()
{
   bool retVal = true;

   // should keep allocation here
   CollectionInfoBuilder cib(Config);

   (void)cib.LoadAllCollections();

   std::vector<CollectionInfo *> collections;
   cib.GetCollections(collections);

   std::vector<CollectionInfo *>::iterator it;
   for(it = collections.begin(); it != collections.end() && retVal; ++it)
   {
      CollectionInfo *info = *it;
      std::string title = info->GetName();
      unsigned long crc32 = CalculateCollectionCrc32(info);

      std::stringstream crcStr;
      crcStr << crc32;

      if(title != "Main")
      {
		  if(CollectionChanged(info, crc32))
		  {
			 std::string msg = "Detected collection \"" + title + "\" has changed (new CRC: " + crcStr.str() + "). Rebuilding database for this collection.";
			 Logger::Write(Logger::ZONE_INFO, "Database", msg);

			 (void)ImportDirectory(info, crc32);
			 retVal = true;
		  }
		  else
		  {
			 std::stringstream ss;
			 std::string msg = "Collection \"" + title + "\" has not changed (CRC: " + crcStr.str() + "). Using existing database settings.";
			 Logger::Write(Logger::ZONE_INFO, "Database", msg);

		  }
      }
      //std::cout << "Importing collection metadata for " << info->GetFullTitle() << " (collections." << info->GetName() << ")" <<  std::endl;
      //ImportMetadata(info);
   }
   Logger::Write(Logger::ZONE_INFO, "Database", "COMPLETE");
   Sleep(1000);
   return retVal;
}

unsigned long CollectionDatabase::CalculateCollectionCrc32(CollectionInfo *info)
{
   unsigned long crc = crc32(0L, Z_NULL, 0);

   // start off by reading all of the contents in the collection configuration folders
   std::string settingsFile = info->GetSettingsPath() + "/Settings.conf";
   crc = CrcFile(settingsFile, crc);


   std::string includeFile = info->GetSettingsPath() + "/Include.txt";
   crc = CrcFile(includeFile, crc);

   std::string excludeFile = info->GetSettingsPath() + "/Exclude.txt";
   crc = CrcFile(excludeFile, crc);

   std::string mamelistFile = info->GetSettingsPath() + "/Mamelist.xml";
   crc = CrcFile(mamelistFile, crc);

   DIR *dp;
   struct dirent *dirp;
   std::string path = info->GetListPath();
   dp = opendir(path.c_str());

   if(dp == NULL)
   {
      Logger::Write(Logger::ZONE_ERROR, "Database", "Could not read directory for caching \"" + info->GetListPath() + "\"");
      return crc;
   }

   std::vector<std::string> extensions;
   info->GetExtensions(extensions);
   std::vector<std::string>::iterator extensionsIt;

   // md5sum each filename for the matching extension
   while((dirp = readdir(dp)) != NULL)
   {
      std::string file = dirp->d_name;
      for(extensionsIt = extensions.begin(); extensionsIt != extensions.end(); ++extensionsIt)
      {
         std::string comparator = "." + *extensionsIt;
         int start = file.length() - comparator.length() + 1;

         if(start >= 0 && file.compare(start, comparator.length(), *extensionsIt) == 0)
         {
            std::string filename = dirp->d_name;
            filename.append("\n");
            crc = crc32(crc, (const unsigned char *)filename.c_str(), (unsigned int)filename.length());
         }
      }
   }

   return crc;
}

unsigned long CollectionDatabase::CrcFile(std::string file, unsigned long crc)
{
   // CRC both the filename and its contents
   crc = crc32(crc, (const unsigned char *)file.c_str(), (unsigned int)file.length());
   std::ifstream ifFile(file.c_str());
   if(ifFile.good())
   {
      std::stringstream ss;
      ss << ifFile.rdbuf();

      crc = crc32(crc, (const unsigned char *)ss.str().c_str(), (unsigned int)ss.str().length());
      Logger::Write(Logger::ZONE_INFO, "Database", "Crcing \"" + file + "\"");
      ifFile.close();
   }

   return crc;
}

bool CollectionDatabase::CollectionChanged(CollectionInfo *info, unsigned long crc32)
{
   bool retVal = true;

   sqlite3 *handle = DBInstance->GetHandle();
   int rc;
   sqlite3_stmt *stmt;

   sqlite3_prepare_v2(handle,
            "SELECT crc32 "
            "FROM Collections WHERE collectionName=? and crc32=?;",
               -1, &stmt, 0);

   sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 2, crc32);

   rc = sqlite3_step(stmt);

   if(rc == SQLITE_ROW)
   {
      retVal = false;
   }

   return retVal;
}

bool CollectionDatabase::SetHidden(std::string collectionName, Item *item, bool hidden)
{
   bool retVal = true;
   char *error = NULL;
   sqlite3 *handle = DBInstance->GetHandle();
   std::string mode = (hidden) ? "hidden":"visible";
   int isHidden = (hidden)?1:0;

   Logger::Write(Logger::ZONE_DEBUG, "Database", "Marking  \"" + item->GetFullTitle() + "\" " + mode);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(handle,
               "UPDATE CollectionItems SET hidden=? WHERE collectionName=? AND name=?;",
               -1, &stmt, 0);

   sqlite3_bind_int(stmt, 1, isHidden);
   sqlite3_bind_text(stmt, 2, collectionName.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 3, item->GetFullTitle().c_str(), -1, SQLITE_TRANSIENT);

   sqlite3_step(stmt);
   sqlite3_finalize(stmt);

   sqlite3_exec(handle, "COMMIT TRANSACTION;", NULL, NULL, &error);

   return retVal;
}

//todo: This file needs MASSIVE REFACTORING!
bool CollectionDatabase::ImportDirectory(CollectionInfo *info, unsigned long crc32)
{
   DIR *dp;
   struct dirent *dirp;
   std::string path = info->GetListPath();
   std::map<std::string, Item *> includeFilter;
   std::map<std::string, Item *> excludeFilter;
   std::map<std::string, Item *> includeList;
   std::map<std::string, Item *> metaList;
   bool retVal = true;
   char *error = NULL;
   sqlite3 *handle = DBInstance->GetHandle();
   std::string includeFile = Configuration::GetAbsolutePath() + "/Collections/" + info->GetName() + "/Include.txt";
   std::string excludeFile = Configuration::GetAbsolutePath() + "/Collections/" + info->GetName() + "/Exclude.txt";
   std::string includeHyperListFile = Configuration::GetAbsolutePath() + "/Collections/" + info->GetName() + "/Include.xml";

   if(!ImportBasicList(info, includeFile, includeFilter))
   {
      ImportHyperList(info, includeHyperListFile, includeFilter);

   }
   //todo: this shouldn't be read twice, perform a copy
   ImportHyperList(info, includeHyperListFile, metaList);

   (void)ImportBasicList(info, excludeFile, excludeFilter);

   dp = opendir(path.c_str());
   std::vector<std::string> extensions;
   info->GetExtensions(extensions);
   std::vector<std::string>::iterator extensionsIt;

   if(dp == NULL)
   {
      Logger::Write(Logger::ZONE_ERROR, "Database", "Could not read directory \"" + info->GetListPath() + "\"");
      //todo: store into a database
   }
   else
   {
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
                     if(includeList.find(basename) == includeList.end())
                     {
                        Item *i = new Item();
                        i->SetFullTitle(file);
                        includeList[basename] = i;
                     }
                     if(metaList.find(basename) == metaList.end())
                     {
                        Item *i = new Item();
                        i->SetFullTitle(file);
                        metaList[basename] = i;
                     }
                  }
               }
            }
         }
      }
   }

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


   Logger::Write(Logger::ZONE_INFO, "Database", "Scanning to import  \"" + path + "\"");
   sqlite3_exec(handle, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &error);

   sqlite3_stmt *stmt;
   sqlite3_prepare_v2(handle,
               "DELETE FROM Collections WHERE collectionName=?;",
               -1, &stmt, 0);
   sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);

   sqlite3_prepare_v2(handle,
               "DELETE FROM CollectionItems WHERE collectionName=?;",
               -1, &stmt, 0);
   sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);

   sqlite3_prepare_v2(handle,
               "DELETE FROM Meta WHERE collectionName=?;",
               -1, &stmt, 0);
   sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_step(stmt);
   sqlite3_finalize(stmt);

   if(sqlite3_exec(handle, "COMMIT TRANSACTION;", NULL, NULL, &error) != SQLITE_OK)
   {
      std::stringstream ss;
      ss << "Updating cache collection failure " << error;
      Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());
      retVal = false;
   }

   std::map<std::string, Item *>::iterator it;

   if(sqlite3_exec(handle, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &error) != SQLITE_OK)
   {
      std::stringstream ss;
      ss << "Delete cache collection failure " << error;
      Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());
      retVal = false;
   }

   for(it = includeList.begin(); it != includeList.end(); it++)
   {
      std::string basename = it->first;
      Item *file = it->second;

      std::string name = file->GetFullTitle();
      Utils::NormalizeBackSlashes(name);
      file->SetFullTitle(name);

      sqlite3_prepare_v2(handle,
                  "INSERT OR REPLACE INTO CollectionItems (collectionName, filePath, name) VALUES (?,?,?);",
                  -1, &stmt, 0);

      sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, file->GetFullTitle().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, basename.c_str(), -1, SQLITE_TRANSIENT);

      //todo: better error handling for all of these messages
      sqlite3_step(stmt);
      sqlite3_finalize(stmt);
   }
   for(it = metaList.begin(); it != metaList.end(); it++)
   {
      std::string basename = it->first;
      Item *file = it->second;

      sqlite3_prepare_v2(handle,
                  "INSERT OR REPLACE INTO Meta (collectionName, name, title, year, manufacturer, cloneOf, players, buttons) VALUES (?,?,?,?,?,?,?,?);",
                  -1, &stmt, 0);

      sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 2, basename.c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 3, basename.c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 4, file->GetYear().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 5, file->GetManufacturer().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 6, file->GetCloneOf().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 7, file->GetNumberPlayersString().c_str(), -1, SQLITE_TRANSIENT);
      sqlite3_bind_text(stmt, 8, file->GetNumberButtonsString().c_str(), -1, SQLITE_TRANSIENT);

      sqlite3_step(stmt);
      sqlite3_finalize(stmt);
   }

   sqlite3_prepare_v2(handle,
               "INSERT OR REPLACE INTO Collections (collectionName, crc32) VALUES (?,?);",
               -1, &stmt, 0);

   sqlite3_bind_text(stmt, 1, info->GetName().c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_int(stmt, 2, crc32);

   sqlite3_step(stmt);
   sqlite3_finalize(stmt);


   if(sqlite3_exec(handle, "COMMIT TRANSACTION;", NULL, NULL, &error) != SQLITE_OK)
   {
      std::stringstream ss;
      ss << "Updating cache collection failure " << error;
      Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());
      retVal = false;
   }

   Logger::Write(Logger::ZONE_INFO, "Database", "Imported files from \"" + path + "\" into database");

   //todo: create a helper method to get this file directly (copy paste hazard)
   std::string mamelistFile = info->GetSettingsPath() + "/Mamelist.xml";
   std::ifstream infile(mamelistFile.c_str());

   if(infile.good())
   {
      Logger::Write(Logger::ZONE_INFO, "Database", "Updating Mamelist metadata for \"" + info->GetName() + "\" (\"" + mamelistFile + "\") into database. This will take a while...");
      MamelistMetadata mld(DBInstance);
      mld.Import(mamelistFile, info->GetName());
   }
   infile.close();


   while(includeList.size() > 0)
   {
      std::map<std::string, Item *>::iterator it = includeList.begin();
      delete it->second;
      includeList.erase(it);
   }
   while(metaList.size() > 0)
   {
      std::map<std::string, Item *>::iterator it = metaList.begin();
      delete it->second;
      metaList.erase(it);
   }
   return retVal;
}

bool CollectionDatabase::ImportBasicList(CollectionInfo *info, std::string file, std::map<std::string, Item *> &list)
{
   bool retVal = false;

   Logger::Write(Logger::ZONE_DEBUG, "Database", "Checking to see if \"" + file + "\" exists");

   std::ifstream includeStream(file.c_str());

   if (includeStream.good())
   {
	  Logger::Write(Logger::ZONE_DEBUG, "Database", "Importing \"" + file + "\"");
      std::string line;

      while(std::getline(includeStream, line))
      {
         if(list.find(line) == list.end())
         {
            Item *i = new Item();
            line.erase( std::remove(line.begin(), line.end(), '\r'), line.end() );

            i->SetFullTitle(line);
            list[line] = i;
            Logger::Write(Logger::ZONE_DEBUG, "Database", "Including \"" + line + "\" (if file exists)");
         }
      }

      retVal = true;
   }

   return retVal;
}
bool CollectionDatabase::ImportHyperList(CollectionInfo *info, std::string hyperlistFile, std::map<std::string, Item *> &list)
{
   bool retVal = false;
   rapidxml::xml_document<> doc;
   std::ifstream file(hyperlistFile.c_str());
   std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

   Logger::Write(Logger::ZONE_DEBUG, "Database", "Checking to see if \"" + hyperlistFile + "\" exists");

   if(!file.good())
   {
      Logger::Write(Logger::ZONE_INFO, "Database", "Could not find HyperList file: " + hyperlistFile);
      return retVal;
   }

   try
   {
	  Logger::Write(Logger::ZONE_INFO, "Database", "Importing: " + hyperlistFile);
      buffer.push_back('\0');

      doc.parse<0>(&buffer[0]);

      rapidxml::xml_node<> *root = doc.first_node("menu");


      if(!root)
      {
         Logger::Write(Logger::ZONE_ERROR, "CollectionDatabase", "Does not appear to be a HyperList file (missing <menu> tag)");
         return NULL;
      }
      else
      {
         for(rapidxml::xml_node<> *game = root->first_node("game"); game; game = game->next_sibling("game"))
         {
            rapidxml::xml_attribute<> *nameXml = game->first_attribute("name");
            rapidxml::xml_node<> *descriptionXml = game->first_node("description");
            rapidxml::xml_node<> *cloneofXml = game->first_node("cloneof");
            rapidxml::xml_node<> *crcXml = game->first_node("crc");
            rapidxml::xml_node<> *manufacturerXml = game->first_node("manufacturer");
            rapidxml::xml_node<> *yearXml = game->first_node("year");
            rapidxml::xml_node<> *genreXml = game->first_node("genre");
            rapidxml::xml_node<> *ratingXml = game->first_node("rating");
            rapidxml::xml_node<> *enabledXml = game->first_node("enabled");
            std::string name = (nameXml) ? nameXml->value() : "";
            std::string description = (descriptionXml) ? descriptionXml->value() : "";
            std::string crc = (crcXml) ? crcXml->value() : "";
            std::string cloneOf = (cloneofXml) ? cloneofXml->value() : "";
            std::string manufacturer = (manufacturerXml) ? manufacturerXml->value() : "";
            std::string year = (yearXml) ? yearXml->value() : "";
            std::string genre = (genreXml) ? genreXml->value() : "";
            std::string rating = (ratingXml) ? ratingXml->value() : "";
            std::string enabled = (enabledXml) ? enabledXml->value() : "";

            if(name.length() > 0 && list.find(name) == list.end())
            {
               Item *i = new Item();
               i->SetFullTitle(name);
               i->SetYear(year);
               i->SetManufacturer(manufacturer);
               i->SetCloneOf(cloneOf);
               list[name] = i;
               Logger::Write(Logger::ZONE_DEBUG, "Database", "Including \"" + name + "\" (if file exists)");
            }

         }
      }
   }
   catch(rapidxml::parse_error &e)
   {
      std::string what = e.what();
      long line = static_cast<long>(std::count(&buffer.front(), e.where<char>(), char('\n')) + 1);
      std::stringstream ss;
      ss << "Could not parse layout file. [Line: " << line << "] Reason: " << e.what();

      Logger::Write(Logger::ZONE_ERROR, "Layout", ss.str());
   }
   catch(std::exception &e)
   {
      std::string what = e.what();
      Logger::Write(Logger::ZONE_ERROR, "Layout", "Could not parse layout file. Reason: " + what);
   }


   return retVal;
}

/*
bool CollectionDatabase::ImportMetadata(CollectionInfo *info)
{
   bool retVal = true;
   std::string type = info->GetMetadataType();

   if(type.compare("mamelist") == 0)
   {
      MamelistMetadata meta;
      //todo: pass in collectionName
      retVal = meta.Import(info->GetMetadataPath(), "arcade");
   }
   else if(!type.empty())
   {
      std::stringstream ss;
      ss << "Unsupported metadata type \"" << type << "\" for " << info->GetFullTitle() << " (collections." << info->GetName() << ".metadata.type)" << std::endl;
   Log::Write(Log::ERROR, "Database", ss.str());

      retVal = false;
   }

   return retVal;
}
*/

bool CollectionDatabase::GetCollection(std::string collectionName, std::vector<Item *> &list)
{
   bool retVal = true;

   sqlite3 *handle = DBInstance->GetHandle();
   int rc;
   sqlite3_stmt *stmt;

   bool showParenthesis = true;
   bool showSquareBrackets = true;

   (void)Config->GetProperty("showParenthesis", showParenthesis);
   (void)Config->GetProperty("showSquareBrackets", showSquareBrackets);

   //todo: program crashes if this query fails
   sqlite3_prepare_v2(handle,
            "SELECT DISTINCT CollectionItems.filePath, CollectionItems.name, Meta.title, Meta.year, Meta.manufacturer, Meta.players, Meta.buttons, Meta.cloneOf "
            "FROM CollectionItems, Meta WHERE CollectionItems.collectionName=? AND Meta.collectionName=? AND CollectionItems.name=Meta.name  AND CollectionItems.hidden=0 ORDER BY title ASC;",
               -1, &stmt, 0);

   sqlite3_bind_text(stmt, 1, collectionName.c_str(), -1, SQLITE_TRANSIENT);
   sqlite3_bind_text(stmt, 2, collectionName.c_str(), -1, SQLITE_TRANSIENT);

   rc = sqlite3_step(stmt);

   while(rc == SQLITE_ROW)
   {
      std::string filePath = (char *)sqlite3_column_text(stmt, 0);
      std::string name = (char *)sqlite3_column_text(stmt, 1);
      std::string fullTitle = (char *)sqlite3_column_text(stmt, 2);
      std::string year = (char *)sqlite3_column_text(stmt, 3);
      std::string manufacturer = (char *)sqlite3_column_text(stmt, 4);
      int numberPlayers = (int)sqlite3_column_int(stmt, 5);
      int numberButtons = (int)sqlite3_column_int(stmt, 6);
      std::string cloneOf = (char *)sqlite3_column_text(stmt, 7);
      std::string launcher;
      std::string title = fullTitle;

      if(!showParenthesis)
      {
         std::string::size_type firstPos  = title.find_first_of("(");
         std::string::size_type secondPos = title.find_first_of(")", firstPos);

         while(firstPos != std::string::npos && secondPos != std::string::npos)
         {
            firstPos  = title.find_first_of("(");
            secondPos = title.find_first_of(")", firstPos);

            if (firstPos != std::string::npos)
            {
                title.erase(firstPos, (secondPos - firstPos) + 1);
            }
         }
      }
      if(!showSquareBrackets)
      {
         std::string::size_type firstPos  = title.find_first_of("[");
         std::string::size_type secondPos = title.find_first_of("]", firstPos);

         while(firstPos != std::string::npos && secondPos != std::string::npos)
         {
            firstPos  = title.find_first_of("[");
            secondPos = title.find_first_of("]", firstPos);

            if (firstPos != std::string::npos && secondPos != std::string::npos)
            {
                title.erase(firstPos, (secondPos - firstPos) + 1);
            }
         }
      }

      Item *item = new Item();
      item->SetFilePath(filePath);
      item->SetName(name);
      item->SetTitle(title);
      item->SetFullTitle(fullTitle);
      item->SetYear(year);
      item->SetManufacturer(manufacturer);
      item->SetNumberPlayers(numberPlayers);
      item->SetNumberButtons(numberButtons);
      item->SetCloneOf(cloneOf);

      //std::cout << "loading " << title << std::endl;
      if(Config->GetProperty("collections." + collectionName + ".launcher", launcher))
      {
    	  item->SetLauncher(launcher);
      }

      list.push_back(item);

      rc = sqlite3_step(stmt);
   }

   //todo: query the metadata table to populate each item

   return retVal;
}
