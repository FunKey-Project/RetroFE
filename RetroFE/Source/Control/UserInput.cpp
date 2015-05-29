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
        retVal = MapKey("left", KeyCodeUp) && retVal;
    }
    if(!MapKey("left", KeyCodeLeft))
    {
        retVal = MapKey("up", KeyCodeLeft) && retVal;
    }
    if(!MapKey("down", KeyCodeDown))
    {
        retVal = MapKey("right", KeyCodeDown) && retVal;
    }
    if(!MapKey("right", KeyCodeRight))
    {
        retVal = MapKey("down", KeyCodeRight) && retVal;
    }

    retVal = MapKey("pageDown", KeyCodePageDown) && retVal;
    retVal = MapKey("pageUp", KeyCodePageUp) && retVal;
    MapKey("letterDown", KeyCodeLetterDown);
    MapKey("letterUp", KeyCodeLetterUp);
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


UserInput::KeyCode_E UserInput::GetKeycode(SDL_Scancode scancode)
{
    KeyCode_E keycode = KeyCodeNull;

    std::map<SDL_Scancode, KeyCode_E>::iterator it = ReverseKeyMap.find(scancode);

    if(it != ReverseKeyMap.end())
    {
        keycode = it->second;
    }

    return keycode;
}

void UserInput::ResetKeyStates()
{
    for(std::map<KeyCode_E, bool>::iterator it = KeyState.begin(); it != KeyState.end(); it++)
    {
      it->second = false;
    }
}


bool UserInput::MapKey(std::string keyDescription, KeyCode_E key)
{
    SDL_Scancode scanCode;
    std::string description;

    std::string configKey = "controls." + keyDescription;

    if(!Config.GetProperty(configKey, description))
    {
        Logger::Write(Logger::ZONE_ERROR, "Configuration", "Missing property " + configKey);
        return false;
    }

    scanCode = SDL_GetScancodeFromName(description.c_str());

    if(scanCode == SDL_SCANCODE_UNKNOWN)
    {
        Logger::Write(Logger::ZONE_ERROR, "Configuration", "Unsupported property value for " + configKey + "(" + description + "). See Documentation/Keycodes.txt for valid inputs");
        return false;
    }

    KeyMap[key] = scanCode;
    ReverseKeyMap[scanCode] = key;
    KeyState[key] = false;
    return true;
}

bool UserInput::SetKeyState(SDL_Scancode code, bool state)
{
    KeyCode_E key = GetKeycode(code);

    if(key == KeyCodeNull) { return false; }
    if(KeyState.find(key) == KeyState.end()) { return false; }

    KeyState[key] = state;
    return true;

}
bool UserInput::GetKeyState(KeyCode_E key)
{
    if(KeyState.find(key) == KeyState.end()) { return false; }
    return KeyState[key];
}



