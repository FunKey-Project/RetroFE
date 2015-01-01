/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "MenuParser.h"
#include "Item.h"
#include "../Utility/Log.h"
#include "../Database/Configuration.h"
#include "../Database/CollectionDatabase.h"
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
bool MenuParser::GetMenuItems(CollectionDatabase *cdb, std::string collectionName, std::vector<Item *> &items)
{
   bool retVal = false;
   //todo: magic string
   std::string menuFilename = Configuration::GetAbsolutePath() + "/Collections/" + collectionName + "/Menu.xml";
   rapidxml::xml_document<> doc;
   rapidxml::xml_node<> * rootNode;

   Logger::Write(Logger::ZONE_INFO, "Menu", "Checking if menu exists at \"" + menuFilename + "\"");

   try
   {
      std::ifstream file(menuFilename.c_str());

      // gracefully exit if there is no menu file for the pa
      if(file.good())
      {
         Logger::Write(Logger::ZONE_INFO, "Menu", "Found menu");
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
            else
            {
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
                  items.push_back(item);
               }
               else
               {
                  std::string collectionName = collectionAttribute->value();
                  Logger::Write(Logger::ZONE_INFO, "Menu", "Loading collection into menu: " + collectionName);
                  cdb->GetCollection(collectionAttribute->value(), items);
               }
            }
         }

         std::sort( items.begin(), items.end(), VectorSort);

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
