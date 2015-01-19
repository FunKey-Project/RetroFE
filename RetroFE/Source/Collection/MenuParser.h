/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

class CollectionInfo;

class MenuParser
{
public:
    MenuParser();
    virtual ~MenuParser();
    bool GetMenuItems(CollectionInfo *cdb);

};
