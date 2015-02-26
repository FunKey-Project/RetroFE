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
#include "../Database/Configuration.h"
#include "../Database/DB.h"
#include <algorithm>
#include <rapidxml.hpp>
#include <fstream>
#include <sstream>

bool VectorSort(const Item *d1, const Item *d2)
{
    return d1->GetLCTitle() < d2->GetLCTitle();
}

MenuParser::MenuParser()
{
}

MenuParser::~MenuParser()
{
}

//todo: clean up this method, too much nesting
bool MenuParser::GetMenuItems(CollectionInfo *collection)
{
    bool retVal = false;
    //todo: magic string
    std::string menuFilename = Configuration::GetAbsolutePath() + "/collections/" + collection->GetName() + "/Menu.xml";
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> * rootNode;

    Logger::Write(Logger::ZONE_INFO, "Menu", "Checking if menu exists at \"" + menuFilename + "\"");

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
                    Logger::Write(Logger::ZONE_ERROR, "Menu", "Menu item tag is missing collection attribute");
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
                    item->SetTitle(title);
                    item->SetFullTitle(title);
                    item->SetName(collectionAttribute->value());
                    item->SetIsLeaf(false);
                    collection->GetItems()->push_back(item);

                }
                else
                {
                    std::string collectionName = collectionAttribute->value();
                    Logger::Write(Logger::ZONE_INFO, "Menu", "Loading collection into menu: " + collectionName);

                    //todo: unsupported option with this refactor
                    // need to append the collection
                }
            }

            // todo: sorting should occur within the collection itself, not externally
            std::vector<Item *> *items = collection->GetItems();
            std::sort( items->begin(), items->end(), VectorSort);

            retVal = true;
        }
    }
    catch(std::ifstream::failure &e)
    {
        std::stringstream ss;
        ss << "Unable to open menu file \"" << menuFilename << "\": " << e.what();
        Logger::Write(Logger::ZONE_ERROR, "Menu", ss.str());
    }

    return retVal;

}
