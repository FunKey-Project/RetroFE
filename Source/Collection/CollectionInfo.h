/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>
#include <vector>

class CollectionInfo
{
public:
   CollectionInfo(std::string name, std::string listPath, std::string extensions, std::string metadataType, std::string metadataPath);
   virtual ~CollectionInfo();
   std::string GetName() const;
   std::string GetSettingsPath() const;
   std::string GetListPath() const;
   std::string GetMetadataType() const;
   std::string GetMetadataPath() const;
   std::string GetExtensions() const;
   void GetExtensions(std::vector<std::string> &extensions);

private:
   std::string Name;
   std::string ListPath;
   std::string Extensions;
   std::string MetadataType;
   std::string MetadataPath;
};
