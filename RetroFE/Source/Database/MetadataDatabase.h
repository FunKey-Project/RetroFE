/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>
#include <vector>
#include <map>

class DB;
class Configuration;
class CollectionInfo;
class Item;

class MetadataDatabase
{
public:
    MetadataDatabase(DB &db, Configuration &c);
    virtual ~MetadataDatabase();
    bool Initialize();
    bool ResetDatabase();

    void UpdateMetadata(CollectionInfo *collection);

private:
    Configuration &Config;
    DB &DBInstance;
};
