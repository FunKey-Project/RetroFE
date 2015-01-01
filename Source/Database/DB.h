/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <sqlite3.h>
#include <string>
class DB
{
public:
   DB();
   bool Initialize();
   void DeInitialize();
   virtual ~DB();
   sqlite3 *GetHandle() { return Handle; }

private:
   sqlite3 *Handle;
   std::string Path;
};

