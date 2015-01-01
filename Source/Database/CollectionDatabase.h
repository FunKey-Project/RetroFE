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

class CollectionDatabase
{
public:
   CollectionDatabase(DB *db, Configuration *c);
   virtual ~CollectionDatabase();
   bool Import();
   bool ResetDatabase();
   bool CheckDatabase();


   bool GetCollection(std::string collectionName, std::vector<Item *> &list);
   bool SetHidden(std::string collectionName, Item *item, bool hidden);

private:
   unsigned long CalculateCollectionCrc32(CollectionInfo *info);
   bool CollectionChanged(CollectionInfo *info, unsigned long crc32);
   unsigned long CrcFile(std::string file, unsigned long crc);

//   bool ImportMetadata(CollectionInfo *info);
   bool ImportDirectory(CollectionInfo *info, unsigned long crc32);
   bool ImportBasicList(CollectionInfo *info,
            std::string file,
            std::map<std::string, Item *> &list);
   bool ImportHyperList(CollectionInfo *info,
            std::string file,
            std::map<std::string, Item *> &list);
   std::map<std::string, Item *> *ImportHyperList(CollectionInfo *info);
   Configuration *Config;
   DB *DBInstance;
};
