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
#include "../Utility/Utils.h"
#include "KeyboardHandler.h"

UserInput::UserInput(Configuration &c)
    : config_(c)
    , joystick_(NULL)
{
    for(unsigned int i = 0; i < KeyCodeMax; ++i)
    {
        keyHandlers_[i] = NULL;
        lastKeyState_[i] = false;
    }
}

UserInput::~UserInput()
{
}

bool UserInput::initialize()
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

    for(int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        joystick_ = SDL_JoystickOpen(i);
        break;
    }
    

    return retVal;
}

bool UserInput::MapKey(std::string keyDescription, KeyCode_E key)
{
    SDL_Scancode scanCode;
    std::string description;

    std::string configKey = "controls." + keyDescription;

    if(!config_.getProperty(configKey, description))
    {
        Logger::write(Logger::ZONE_ERROR, "Input", "Missing property " + configKey);
        return false;
    }


    scanCode = SDL_GetScancodeFromName(description.c_str());

    if(scanCode != SDL_SCANCODE_UNKNOWN)
    {
        Logger::write(Logger::ZONE_INFO, "Input", "Binding key " + configKey);
        keyHandlers_[key] = new KeyboardHandler(scanCode);
        return true;
    }
/*
    else if(description.find("joy") == 0)
    {
        std::string joydesc = Utils::replace(Utils::toLower(description), "joy", "");
        if(joydesc.find("button"))
        {
            unsigned int button;
            std::stringstream ss;
            ss << Utils::replace(joydesc, "button", "");
            ss >> button;
            buttonMap_[key] = button;
            keyState_[key] = 0;
            Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad button " + ss.str() );
        }
        else if(joydesc.find("axis"))
        {
            unsigned int axis;
            std::stringstream ss;
            ss << Utils::replace(joydesc, "axis", "");
            ss >> axis;
            buttonMap_[key] = axis;
            keyState_[key] = 0;
            Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad axis " + ss.str() );
            keyCallbacks_[key] = new JoyAxisHandler();
        }
        else if(joydesc.find("hat"))
        {
            joydesc = Utils::replace(joydesc, "hat", "");
            if(joydesc == "leftup") hatMap_[key] = SDL_HAT_LEFTUP;
            else if(joydesc == "left") hatMap_[key] = SDL_HAT_LEFT;
            else if(joydesc == "leftdown") hatMap_[key] = SDL_HAT_LEFT_DOWN;
            else if(joydesc == "up") hatMap_[key] = SDL_HAT_UP;
            //else if(joydesc == "centered") hatMap_[key] = SDL_HAT_CENTERED;
            else if(joydesc == "down") hatMap_[key] = SDL_HAT_DOWN;
            else if(joydesc == "rightup") hatMap_[key] = SDL_HAT_RIGHTUP;
            else if(joydesc == "right") hatMap_[key] = SDL_HAT_RIGHT;
            else if(joydesc == "rightdown") hatMap_[key] = SDL_HAT_RIGHTDOWN;
            keyState_[key] = 0;
            Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad hat " + joydesc );
        }
    }
*/
    Logger::write(Logger::ZONE_ERROR, "Input", "Unsupported property value for " + configKey + "(" + description + "). See Documentation/Keycodes.txt for valid inputs");
    return false;
}

void UserInput::resetStates()
{
    for(unsigned int i = 0; i < KeyCodeMax; ++i)
    {
        keyHandlers_[i]->reset();
    }
}
bool UserInput::update(SDL_Event &e)
{
    bool updated = false;
    for(unsigned int i = 0; i < KeyCodeMax; ++i)
    {
        InputHandler *h = keyHandlers_[i];
        if(h)
        {
            if(h->update(e)) updated = true;

            lastKeyState_[i] = h->pressed();
        }
    }
    
    return updated;
}

bool UserInput::keystate(KeyCode_E code)
{
    return lastKeyState_[code];
}
