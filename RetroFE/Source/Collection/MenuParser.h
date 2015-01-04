/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once
#include "Item.h"
#include <vector>

class CollectionDatabase;

class MenuParser
{
public:
    MenuParser();
    virtual ~MenuParser();
    bool GetMenuItems(CollectionDatabase *cdb, std::string collectionName, std::vector<Item *> &items);

};
