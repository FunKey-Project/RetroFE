/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "CollectionInfo.h"
#include "../Database/Configuration.h"
#include <sstream>

CollectionInfo::CollectionInfo(std::string name, 
                               std::string listPath, 
                               std::string extensions, 
                               std::string metadataType, 
                               std::string metadataPath)
: Name(name)
, ListPath(listPath)
, Extensions(extensions)
, MetadataType(metadataType)
, MetadataPath(metadataPath)
{
}

CollectionInfo::~CollectionInfo()
{
}

std::string CollectionInfo::GetName() const
{
   return Name;
}

std::string CollectionInfo::GetSettingsPath() const
{
   return Configuration::GetAbsolutePath() + "/Collections/" + GetName();
}

std::string CollectionInfo::GetListPath() const
{
   return ListPath;
}

std::string CollectionInfo::GetMetadataType() const
{
   return MetadataType;
}

std::string CollectionInfo::GetMetadataPath() const
{
   return MetadataPath;
}

std::string CollectionInfo::GetExtensions() const
{
   return Extensions;
}

void CollectionInfo::GetExtensions(std::vector<std::string> &extensions)
{
   std::istringstream ss(Extensions);
   std::string token;

   while(std::getline(ss, token, ','))
   {
      extensions.push_back(token);
   }
}



