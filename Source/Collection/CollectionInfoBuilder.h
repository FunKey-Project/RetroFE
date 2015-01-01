/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>
#include <map>
#include <vector>

class Configuration;
class CollectionInfo;

class CollectionInfoBuilder
{
public:
    CollectionInfoBuilder(Configuration *c);
    virtual ~CollectionInfoBuilder();
    bool LoadAllCollections();
    void GetCollections(std::vector<CollectionInfo *> &keys);

private:
    bool ImportCollection(std::string name);
    std::map<std::string, CollectionInfo *> InfoMap;
    Configuration *Conf;
};
