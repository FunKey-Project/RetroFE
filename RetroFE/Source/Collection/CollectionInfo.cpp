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



