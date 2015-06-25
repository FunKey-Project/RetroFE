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

#include "MenuParser.h"
#include "CollectionInfo.h"
#include "Item.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include "../Database/Configuration.h"
#include "../Database/DB.h"
#include <algorithm>
#include <rapidxml.hpp>
#include <fstream>
#include <sstream>

bool VectorSort(Item *d1, Item *d2)
{
    return d1->lowercaseTitle() < d2->lowercaseTitle();
}

MenuParser::MenuParser()
{
}

MenuParser::~MenuParser()
{
}

//todo: clean up this method, too much nesting
bool MenuParser::menuItems(CollectionInfo *collection)
{
    bool retVal = false;
    //todo: magic string
    std::string menuFilename = Utils::combinePath(Configuration::absolutePath, "collections", collection->name, "menu.xml");
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> * rootNode;

    Logger::write(Logger::ZONE_INFO, "Menu", "Checking if menu exists at \"" + menuFilename + "\"");

    try
    {
        std::ifstream file(menuFilename.c_str());

        // gracefully exit if there is no menu file for the pa
        if(file.good())
        {
            std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            buffer.push_back('\0');

            doc.parse<0>(&buffer[0]);

            rootNode = doc.first_node("menu");

            for (rapidxml::xml_node<> * itemNode = rootNode->first_node("item"); itemNode; itemNode = itemNode->next_sibling())
            {
                rapidxml::xml_attribute<> *collectionAttribute = itemNode->first_attribute("collection");
                rapidxml::xml_attribute<> *importAttribute = itemNode->first_attribute("import");

                if(!collectionAttribute)
                {
                    retVal = false;
                    Logger::write(Logger::ZONE_ERROR, "Menu", "Menu item tag is missing collection attribute");
                    break;
                }
                //todo: too much nesting! Ack!
                std::string import;
                if(importAttribute)
                {
                    import = importAttribute->value();
                }

                if(import != "true")
                {
                    //todo, check for empty string
                    std::string title = collectionAttribute->value();
                    Item *item = new Item();
                    item->title = title;
                    item->fullTitle = title;
                    item->name = collectionAttribute->value();
                    item->leaf = false;
                    collection->items.push_back(item);

                }
                else
                {
                    std::string collectionName = collectionAttribute->value();
                    Logger::write(Logger::ZONE_INFO, "Menu", "Loading collection into menu: " + collectionName);

                    //todo: unsupported option with this refactor
                    // need to append the collection
                }
            }

            // todo: sorting should occur within the collection itself, not externally
            std::vector<Item *> *items = &collection->items;
            std::sort( items->begin(), items->end(), VectorSort);

            retVal = true;
        }
    }
    catch(std::ifstream::failure &e)
    {
        std::stringstream ss;
        ss << "Unable to open menu file \"" << menuFilename << "\": " << e.what();
        Logger::write(Logger::ZONE_ERROR, "Menu", ss.str());
    }

    return retVal;

}
