/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "../Collection/Item.h"

class MenuNotifierInterface
{
public:
   virtual ~MenuNotifierInterface() {}
   virtual void OnNewItemSelected(Item *) = 0;
};

