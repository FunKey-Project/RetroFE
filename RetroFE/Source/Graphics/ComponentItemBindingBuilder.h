/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <vector>
#include <string>

class Item;
class ComponentItemBinding;

class ComponentItemBindingBuilder
{
public:
    ComponentItemBindingBuilder();
    virtual ~ComponentItemBindingBuilder();
    static std::vector<ComponentItemBinding *> *BuildCollectionItems(std::vector<Item *> *infoList);
};
