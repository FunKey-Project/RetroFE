/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "MamelistMetadata.h"
#include "DB.h"
#include "../Utility/Log.h"
#include "Metadata.h"
#include <rapidxml.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <sqlite3.h>


MamelistMetadata::MamelistMetadata(DB *dbInstance)
: DBInstance(dbInstance)
{
}

MamelistMetadata::~MamelistMetadata()
{
}

bool MamelistMetadata::Import(std::string filename, std::string collection)
{
   bool retVal = true;
   rapidxml::xml_document<> doc;
   rapidxml::xml_node<> * rootNode;
   char *error = NULL;
   sqlite3 *handle = DBInstance->GetHandle();

   std::ifstream f(filename.c_str());

   if (!f.good())
   {
      Logger::Write(Logger::ZONE_ERROR, "Mamelist", "Could not find mamelist metadata file at \"" + filename + "\"");

      retVal = false;
   }

   f.close();

   if(retVal)
   {
	  Logger::Write(Logger::ZONE_INFO, "Mamelist", "Importing mamelist file \"" + filename + "\"");
      std::ifstream file(filename.c_str());
      std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

      buffer.push_back('\0');

      doc.parse<0>(&buffer[0]);

      rootNode = doc.first_node("mame");

      sqlite3_exec(handle, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &error);
      for (rapidxml::xml_node<> * game = rootNode->first_node("game"); game; game = game->next_sibling())
      {
          rapidxml::xml_attribute<> *nameNode = game->first_attribute("name");
          rapidxml::xml_attribute<> *cloneOfXml = game->first_attribute("cloneof");

         if(nameNode != NULL)
         {
           std::string name = nameNode->value();
           rapidxml::xml_node<> *descriptionNode = game->first_node("description");
           rapidxml::xml_node<> *yearNode = game->first_node("year");
           rapidxml::xml_node<> *manufacturerNode = game->first_node("manufacturer");
           rapidxml::xml_node<> *inputNode = game->first_node("input");

           std::string description = (descriptionNode == NULL) ? nameNode->value() : descriptionNode->value();
           std::string year = (yearNode == NULL) ? "" : yearNode->value();
           std::string manufacturer = (manufacturerNode == NULL) ? "" : manufacturerNode->value();
           std::string cloneOf = (cloneOfXml == NULL) ? "" : cloneOfXml->value();
           std::string players;
           std::string buttons;

           if(inputNode != NULL)
           {
              rapidxml::xml_attribute<> *playersAttribute = inputNode->first_attribute("players");
              rapidxml::xml_attribute<> *buttonsAttribute = inputNode->first_attribute("buttons");

              if(playersAttribute)
              {
                 players = playersAttribute->value();
              }

              if(buttonsAttribute)
              {
                 buttons = buttonsAttribute->value();
              }

           }

           sqlite3_stmt *stmt;
           sqlite3_prepare_v2(handle,
                       "UPDATE OR REPLACE Meta SET title=?, year=?, manufacturer=?, players=?, buttons=?, cloneOf=? WHERE name=? AND collectionName=?;",
                       -1, &stmt, 0);

           sqlite3_bind_text(stmt, 1, description.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 2, year.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 3, manufacturer.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 4, players.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 5, buttons.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 6, cloneOf.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 7, name.c_str(), -1, SQLITE_TRANSIENT);
           sqlite3_bind_text(stmt, 8, collection.c_str(), -1, SQLITE_TRANSIENT);

           sqlite3_step(stmt);
           sqlite3_finalize(stmt);

         }
      }
      sqlite3_exec(handle, "COMMIT TRANSACTION;", NULL, NULL, &error);

   }



   return retVal;
}
