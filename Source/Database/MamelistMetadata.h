/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Metadata.h"

class DB;

class MamelistMetadata : Metadata
{
public:
   MamelistMetadata(DB *dbInstance);
   virtual ~MamelistMetadata();
   bool Import(std::string file, std::string collectionName);
private:
   DB *DBInstance;
};
