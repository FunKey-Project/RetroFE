/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "../Database/DB.h"
#include "../Database/MetadataDatabase.h"
#include <string>
#include <map>
#include <vector>

class Configuration;
class CollectionInfo;


class CollectionInfoBuilder
{
public:
    CollectionInfoBuilder(Configuration &c, DB &db);
    virtual ~CollectionInfoBuilder();
    CollectionInfo *BuildCollection(std::string collectionName);

private:
    MetadataDatabase MetaDB;
    bool ImportDirectory(CollectionInfo *info);
    Configuration &Conf;
};
