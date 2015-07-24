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
#include "JoyAxisHandler.h"
#include "JoyButtonHandler.h"
#include "JoyHatHandler.h"
#include "KeyboardHandler.h"
#include "MouseButtonHandler.h"

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
    
    description = Utils::toLower(description);

    if(description.find("mouse") == 0)
    {
        std::string mousedesc = Utils::replace(Utils::toLower(description), "mouse", "");
        if(mousedesc.find("button") == 0)
        {
            int button = 0;
            std::stringstream ss;
            mousedesc = Utils::replace(mousedesc, "button", "");
            if(mousedesc == "left") button = SDL_BUTTON_LEFT;
            else if(mousedesc == "middle") button = SDL_BUTTON_MIDDLE;
            else if(mousedesc == "right") button = SDL_BUTTON_RIGHT;
            else if(mousedesc == "x1") button = SDL_BUTTON_X1;
            else if(mousedesc == "x2") button = SDL_BUTTON_X2;
            
            keyHandlers_[key] = new MouseButtonHandler(button);
            Logger::write(Logger::ZONE_INFO, "Input", "Binding mouse button " + ss.str() );
            return true;
        }
    }
    else if(description.find("joy") == 0)
    {
        std::string joydesc = Utils::replace(Utils::toLower(description), "joy", "");
        if(joydesc.find("button") == 0)
        {
            unsigned int button;
            std::stringstream ss;
            ss << Utils::replace(joydesc, "button", "");
            ss >> button;
            keyHandlers_[key] = new JoyButtonHandler(button);
            Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad button " + ss.str() );
            return true;
        }
        else if(joydesc.find("hat"))
        {
            Uint8 hat;

            joydesc = Utils::replace(joydesc, "hat", "");
            if(joydesc == "leftup") hat = SDL_HAT_LEFTUP;
            else if(joydesc == "left") hat = SDL_HAT_LEFT;
            else if(joydesc == "leftdown") hat = SDL_HAT_LEFTDOWN;
            else if(joydesc == "up") hat = SDL_HAT_UP;
            //else if(joydesc == "centered") hat = SDL_HAT_CENTERED;
            else if(joydesc == "down") hat = SDL_HAT_DOWN;
            else if(joydesc == "rightup") hat = SDL_HAT_RIGHTUP;
            else if(joydesc == "right") hat = SDL_HAT_RIGHT;
            else if(joydesc == "rightdown") hat = SDL_HAT_RIGHTDOWN;

            keyHandlers_[key] = new JoyHatHandler(hat);
            Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad hat " + joydesc );
            return true;
        }
        else if(joydesc.find("axis"))
        {
            // string is now axis0+
            unsigned int axis;
            Sint16 min;
            Sint16 max;
            Utils::replace(joydesc, "axis", "");

            // string is now 0+
            if(joydesc.find("-") != std::string::npos)
            {
                min = -32768;
                max = -1000;
                Utils::replace(joydesc, "-", "");
            }
            else if(joydesc.find("+") != std::string::npos)
            {
                min = 1000;
                max = 32767;
                Utils::replace(joydesc, "+", "");
            }

            // string is now just the axis number
            std::stringstream ss;
            ss << joydesc;
            ss >> axis;
            Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad axis " + ss.str() );
            keyHandlers_[key] = new JoyAxisHandler(axis, min, max);
            return true;
        }
    }
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
