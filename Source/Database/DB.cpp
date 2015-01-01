/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "DB.h"
#include "Configuration.h"
#include "../Utility/Log.h"

#include <sstream>
#include <fstream>

DB::DB()
: Path(Configuration::GetAbsolutePath() + "/cache.db")
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

