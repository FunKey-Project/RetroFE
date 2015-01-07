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
#include "DB.h"
#include "../Utility/Log.h"

#include <sstream>
#include <fstream>

DB::DB(std::string dbFile)
    : Path(dbFile)
    , Handle(NULL)
{
}

DB::~DB()
{
    DeInitialize();
}

bool DB::Initialize()
{
    bool retVal = false;

    if(sqlite3_open(Path.c_str(), &Handle) != 0)
    {
        std::stringstream ss;
        ss << "Cannot open database: \"" << Path << "\"" << sqlite3_errmsg(Handle);
        Logger::Write(Logger::ZONE_ERROR, "Database", ss.str());
    }
    else
    {
        Logger::Write(Logger::ZONE_INFO, "Database", "Opened database \"" + Path + "\"");
        retVal = true;
    }

    return retVal;
}


void DB::DeInitialize()
{
    if(Handle != NULL)
    {
        sqlite3_close(Handle);
        Handle = NULL;
    }
}

