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

#include "UserInput.h"
#include "../Database/Configuration.h"
#include "../Utility/Log.h"

UserInput::UserInput(Configuration &c)
    : Config(c)
{
}

UserInput::~UserInput()
{
}

bool UserInput::Initialize()
{
    bool retVal = true;

    if(!MapKey("up", KeyCodeUp))
    {
        retVal = MapKey("previousItem", KeyCodeUp) && retVal;
    }
    if(!MapKey("left", KeyCodeLeft))
    {
        retVal = MapKey("previousItem", KeyCodeLeft) && retVal;
    }
    if(!MapKey("down", KeyCodeDown))
    {
        retVal = MapKey("nextItem", KeyCodeDown) && retVal;
    }
    if(!MapKey("right", KeyCodeRight))
    {
        retVal = MapKey("nextItem", KeyCodeRight) && retVal;
    }

    retVal = MapKey("pageDown", KeyCodePageDown) && retVal;
    retVal = MapKey("pageUp", KeyCodePageUp) && retVal;
    retVal = MapKey("select", KeyCodeSelect) && retVal;
    retVal = MapKey("back", KeyCodeBack) && retVal;
    retVal = MapKey("quit", KeyCodeQuit) && retVal;
    // these features will need to be implemented at a later time
//   retVal = MapKey("admin", KeyCodeAdminMode) && retVal;
//   retVal = MapKey("remove", KeyCodeHideItem) && retVal;

    return retVal;
}

SDL_Scancode UserInput::GetScancode(KeyCode_E key)
{
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    std::map<KeyCode_E, SDL_Scancode>::iterator it = KeyMap.find(key);

    if(it != KeyMap.end())
    {
        scancode = it->second;
    }

    return scancode;
}


bool UserInput::MapKey(std::string keyDescription, KeyCode_E key)
{
    bool retVal = false;
    SDL_Scancode scanCode;
    std::string description;

    std::string configKey = "controls." + keyDescription;

    if(!Config.GetProperty(configKey, description))
    {
        Logger::Write(Logger::ZONE_ERROR, "Configuration", "Missing property " + configKey);
    }
    else
    {
        scanCode = SDL_GetScancodeFromName(description.c_str());

        if(scanCode == SDL_SCANCODE_UNKNOWN)
        {
            Logger::Write(Logger::ZONE_ERROR, "Configuration", "Unsupported property value for " + configKey + "(" + description + "). See Documentation/Keycodes.txt for valid inputs");
        }
        else
        {
            KeyMap[key] = scanCode;
            retVal = true;
        }
    }

    return retVal;
}

