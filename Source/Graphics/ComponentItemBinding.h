/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Component/Component.h"
#include "../Collection/Item.h"

class ComponentItemBinding
{
public:
   ComponentItemBinding(Component *c, Item *item);
   ComponentItemBinding(Item *item);
   virtual ~ComponentItemBinding();
   Item* GetCollectionItem() const;

   void SetComponent(Component *c);
   Component* GetComponent() const;

private:
   Component *CollectionComponent;
   Item *CollectionItem;
};
