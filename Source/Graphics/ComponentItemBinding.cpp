/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "ComponentItemBinding.h"

ComponentItemBinding::ComponentItemBinding( Component *c, Item *item)
: CollectionComponent(c)
, CollectionItem(item)
{
}

ComponentItemBinding::ComponentItemBinding(Item *item)
: CollectionComponent(NULL)
, CollectionItem(item)
{
}

ComponentItemBinding::~ComponentItemBinding()
{
}

Item* ComponentItemBinding::GetCollectionItem() const
{
   return CollectionItem;
}

void ComponentItemBinding::SetComponent(Component *c)
{
   CollectionComponent = c;
}

Component* ComponentItemBinding::GetComponent() const
{
   return CollectionComponent;
}
