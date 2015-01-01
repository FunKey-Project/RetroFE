/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "ComponentItemBindingBuilder.h"
#include "ComponentItemBinding.h"
#include "../Database/CollectionDatabase.h"
#include "../Collection/Item.h"

ComponentItemBindingBuilder::ComponentItemBindingBuilder()
{
}

ComponentItemBindingBuilder::~ComponentItemBindingBuilder()
{
}

std::vector<ComponentItemBinding *> *ComponentItemBindingBuilder::BuildCollectionItems(std::vector<Item *> *infoList)
{
    std::vector<ComponentItemBinding *> *sprites = new std::vector<ComponentItemBinding *>();
    std::vector<Item *>::iterator it;

    for(it = infoList->begin(); it != infoList->end(); ++it)
    {
        ComponentItemBinding *s = new ComponentItemBinding(*it);
        sprites->push_back(s);
    }

    return sprites;
}
