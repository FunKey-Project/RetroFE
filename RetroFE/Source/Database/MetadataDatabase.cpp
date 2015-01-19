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
#include "MetadataDatabase.h"
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

MetadataDatabase::MetadataDatabase(DB &db, Configuration &c)
    : Config(c)
    , DBInstance(db)
{

}

MetadataDatabase::~MetadataDatabase()
{
}

bool MetadataDatabase::ResetDatabase()
{
    bool retVal = true;
    int rc;
    char *error = NULL;
    sqlite3 *handle = DBInstance.GetHandle();

    Logger::Write(Logger::ZONE_INFO, "Database", "Erasing");

    std::string sql;
    sql.append("DROP TABLE IF EXISTS Meta;");

    rc = sqlite3_exec(handle, sql.c_str(), NULL, 0, &error);

    if(rc != SQLITE_OK)
    {
        std::stringstream ss;
        ss << "Unable to create Metadata table. Error: " << error;
        Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());
        retVal = false;
    }

    return retVal;
}

bool MetadataDatabase::Initialize()
{
    int rc;
    char *error = NULL;
    sqlite3 *handle = DBInstance.GetHandle();

    std::string sql;
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

    rc = sqlite3_exec(handle, sql.c_str(), NULL, 0, &error);

    if(rc != SQLITE_OK)
    {
        std::stringstream ss;
        ss << "Unable to create Metadata table. Error: " << error;
        Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());

        return false;
    }

    return true;
}

void MetadataDatabase::UpdateMetadata(CollectionInfo *collection)
{
    sqlite3 *handle = DBInstance.GetHandle();
    int rc;
    sqlite3_stmt *stmt;

    bool showParenthesis = true;
    bool showSquareBrackets = true;

    (void)Config.GetProperty("showParenthesis", showParenthesis);
    (void)Config.GetProperty("showSquareBrackets", showSquareBrackets);


    // items into a hash to make it easily searchable
    std::vector<Item *> *items = collection->GetItems();
    std::map<std::string, Item *> itemMap;

    for(std::vector<Item *>::iterator it = items->begin(); it != items->end(); it++)
    {
        itemMap[(*it)->GetName()] = *it;
    }

    //todo: program crashes if this query fails
    sqlite3_prepare_v2(handle,
                       "SELECT DISTINCT Meta.name, Meta.title, Meta.year, Meta.manufacturer, Meta.players, Meta.buttons, Meta.cloneOf "
                       "FROM Meta WHERE collectionName=? ORDER BY title ASC;",
                       -1, &stmt, 0);

    sqlite3_bind_text(stmt, 1, collection->GetName().c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    while(rc == SQLITE_ROW)
    {
        std::string name = (char *)sqlite3_column_text(stmt, 0);
        std::string fullTitle = (char *)sqlite3_column_text(stmt, 1);
        std::string year = (char *)sqlite3_column_text(stmt, 2);
        std::string manufacturer = (char *)sqlite3_column_text(stmt, 3);
        int numberPlayers = (int)sqlite3_column_int(stmt, 4);
        int numberButtons = (int)sqlite3_column_int(stmt, 5);
        std::string cloneOf = (char *)sqlite3_column_text(stmt, 6);
        std::string launcher;
        std::string title = fullTitle;

        //todo: this should be a helper method, peformed both in CollectionInfoBuilder
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

        std::map<std::string, Item *>::iterator it = itemMap.find(name);
        
        if(it != itemMap.end())
        {
            Item *item = it->second;
            item->SetTitle(title);
            item->SetFullTitle(fullTitle);
            item->SetYear(year);
            item->SetManufacturer(manufacturer);
            item->SetNumberPlayers(numberPlayers);
            item->SetNumberButtons(numberButtons);
            item->SetCloneOf(cloneOf);
        }
        rc = sqlite3_step(stmt);
    }
}
