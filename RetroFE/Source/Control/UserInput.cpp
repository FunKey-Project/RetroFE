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
#include <string>
//#include "JoyAxisHandler.h"
//#include "JoyButtonHandler.h"
//#include "JoyHatHandler.h"
#include "KeyboardHandler.h"
#include "MouseButtonHandler.h"

const key_names_s key_names[] = {
/*
  a subset of
  sed 's/^#define \([^ \t]\+\)[ \t]*\([^\ \t]\+\)/  { \"\1\",\t\2 },/' /usr/include/linux/input.h
 */
  { "KEY_RESERVED",	SDLK_UNKNOWN },
  { "KEY_FIRST", SDLK_FIRST },
  { "Escape",	SDLK_ESCAPE },
  { "1",	SDLK_1 },
  { "2",	SDLK_2 },
  { "3",	SDLK_3 },
  { "4",	SDLK_4 },
  { "5",	SDLK_5 },
  { "6",	SDLK_6 },
  { "7",	SDLK_7 },
  { "8",	SDLK_8 },
  { "9",	SDLK_9 },
  { "0",	SDLK_0 },
  { "-",	SDLK_MINUS },
  { "=",	SDLK_EQUALS },
  { "Backspace",	SDLK_BACKSPACE },
  { "Tab",	SDLK_TAB },
  { "Q",	SDLK_q },
  { "W",	SDLK_w },
  { "E",	SDLK_e },
  { "R",	SDLK_r },
  { "T",	SDLK_t },
  { "Y",	SDLK_y },
  { "U",	SDLK_u },
  { "I",	SDLK_i },
  { "O",	SDLK_o },
  { "P",	SDLK_p },
  { "[",	SDLK_LEFTBRACKET },
  { "]",	SDLK_RIGHTBRACKET },
  { "Return",	SDLK_RETURN },
  { "Left Ctrl",	SDLK_LCTRL },
  { "A",	SDLK_a },
  { "S",	SDLK_s },
  { "D",	SDLK_d },
  { "F",	SDLK_f },
  { "G",	SDLK_g },
  { "H",	SDLK_h },
  { "J",	SDLK_j },
  { "K",	SDLK_k },
  { "L",	SDLK_l },
  { ";",	SDLK_SEMICOLON },
  { "'",	SDLK_QUOTE },
  { "Left Shift",	SDLK_LSHIFT },
  { "\\",	SDLK_BACKSLASH },
  { "Z",	SDLK_z },
  { "X",	SDLK_x },
  { "C",	SDLK_c },
  { "V",	SDLK_v },
  { "B",	SDLK_b },
  { "N",	SDLK_n },
  { "M",	SDLK_m },
  { ",",	SDLK_PERIOD },
  { ".",	SDLK_COMMA },
  { "/",	SDLK_SLASH },
  { "Right Shift",	SDLK_RSHIFT },
  { "Left Alt",	SDLK_LALT },
  { "Space",	SDLK_SPACE },
  { "CapsLock",	SDLK_CAPSLOCK },
  { "F1",	SDLK_F1 },
  { "F2",	SDLK_F2 },
  { "F3",	SDLK_F3 },
  { "F4",	SDLK_F4 },
  { "F5",	SDLK_F5 },
  { "F6",	SDLK_F6 },
  { "F7",	SDLK_F7 },
  { "F8",	SDLK_F8 },
  { "F9",	SDLK_F9 },
  { "F10",	SDLK_F10 },
  { "F11",	SDLK_F11 },
  { "F12",	SDLK_F12 },
  { "Right Ctrl",	SDLK_RCTRL },
  { "Right Alt",	SDLK_RALT },
  { "Up",	SDLK_UP },
  { "PageUp",	SDLK_PAGEUP },
  { "Left",	SDLK_LEFT },
  { "Right",	SDLK_RIGHT },
  { "End",	SDLK_END },
  { "Down",	SDLK_DOWN },
  { "PageDown",	SDLK_PAGEDOWN },
  { "Insert",	SDLK_INSERT },
  { "Delete",	SDLK_DELETE },


  { "LAST", -1 },
};


UserInput::UserInput(Configuration &c)
    : config_(c)
{
    for(unsigned int i = 0; i < KeyCodeMax; ++i)
    {
        currentKeyState_[i] = false;
        lastKeyState_[i] = false;
    }
    /*for ( unsigned int i = 0; i < cMaxJoy; i++ )
    {
        joysticks_[i] = -1;
    }*/
}

UserInput::~UserInput()
{
    for (unsigned int i = 0; i < keyHandlers_.size(); ++i)
    {
        if (keyHandlers_[i].first)
        {
            delete keyHandlers_[i].first;
        }
    }
}

bool UserInput::initialize()
{

    // Optional keys
    MapKey("pageDown", KeyCodePageDown, false );
    MapKey("pageUp", KeyCodePageUp, false );
    MapKey("letterDown", KeyCodeLetterDown, false);
    MapKey("letterUp", KeyCodeLetterUp, false);
    MapKey("favPlaylist", KeyCodeFavPlaylist, false);
    MapKey("nextPlaylist", KeyCodeNextPlaylist, false);
    MapKey("prevPlaylist", KeyCodePrevPlaylist, false);
    MapKey("addPlaylist", KeyCodeAddPlaylist, false);
    MapKey("removePlaylist", KeyCodeRemovePlaylist, false);
    MapKey("random", KeyCodeRandom, false);

    bool retVal = true;

    // At least have controls for either a vertical or horizontal menu
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
        retVal = MapKey("right", KeyCodeDown ) && retVal;
    }
    if(!MapKey("right", KeyCodeRight ))
    {
        retVal = MapKey("down", KeyCodeRight ) && retVal;
    }

    // These keys are mandatory
    retVal = MapKey("select", KeyCodeSelect) && retVal;
    retVal = MapKey("back",   KeyCodeBack) && retVal;
    retVal = MapKey("quit",   KeyCodeQuit) && retVal;
    retVal = MapKey("menu",   KeyCodeMenu) && retVal;

    return retVal;
}

/* Return linux Key code from corresponding str*/
SDLKey UserInput::SDL_GetScancodeFromName(const char *name)
{
	int i=0;
	SDLKey returnValue = SDLK_UNKNOWN;
	while(key_names[i].code >= 0){
		if(!strncmp(key_names[i].name, name, 32)){
			returnValue = (SDLKey) key_names[i].code;
			break;
		}
		i++;
	}
	if(key_names[i].code < 0){
		returnValue = SDLK_UNKNOWN;
	}
	return returnValue;
}

bool UserInput::MapKey(std::string keyDescription, KeyCode_E key)
{
    return MapKey(keyDescription, key, true);
}

bool UserInput::MapKey(std::string keyDescription, KeyCode_E key, bool required)
{
	SDLKey scanCode;
    std::string description;

    std::string configKey = "controls." + keyDescription;

    if(!config_.getProperty(configKey, description))
    {
        Logger::Zone zone = (required) ? Logger::ZONE_ERROR : Logger::ZONE_INFO;
        Logger::write(zone, "Input", "Missing property " + configKey);
        return false;
    }

    std::istringstream ss(description);
    std::string token;

    bool success = true;

    while (std::getline(ss, token, ','))
    {
        token = Configuration::trimEnds(token);
        if (token == "" && description != "") // Allow "," as input key
            token = ",";
        scanCode = SDL_GetScancodeFromName(token.c_str());

        bool found = false;

        if (scanCode != SDLK_UNKNOWN || !strcmp(token.c_str(),"SDLK_FIRST") )
        {
            Logger::write(Logger::ZONE_INFO, "Input", "Binding key " + configKey + ", Key Value: " + std::to_string(scanCode));
            keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new KeyboardHandler(scanCode), key));
            found = true;
        }
        else
        {
            token = Utils::toLower(token);

            if (token.find("mouse") == 0)
            {
                std::string mousedesc = Utils::replace(Utils::toLower(token), "mouse", "");
                if (mousedesc.find("button") == 0)
                {
                    int button = 0;
                    std::stringstream ss;
                    mousedesc = Utils::replace(mousedesc, "button", "");
                    if (mousedesc == "left") button = SDL_BUTTON_LEFT;
                    else if (mousedesc == "middle") button = SDL_BUTTON_MIDDLE;
                    else if (mousedesc == "right") button = SDL_BUTTON_RIGHT;
                    else if (mousedesc == "x1") button = SDL_BUTTON_X1;
                    else if (mousedesc == "x2") button = SDL_BUTTON_X2;

                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new MouseButtonHandler(button), key));
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding mouse button " + ss.str());
                    found = true;
                }
            }
            else if (token.find("joy") == 0)
            {
                /*std::string joydesc = Utils::replace(Utils::toLower(token), "joy", "");
                int joynum;
                if ( isdigit( joydesc.at( 0 ) ) )
                {
                    std::stringstream ssjoy;
                    ssjoy << joydesc.at(0);
                    ssjoy >> joynum;
                    joydesc = joydesc.erase(0, 1);
                }
                else
                {
                    joynum = -1;
                }

                if (joydesc.find("button") == 0)
                {
                    unsigned int button;
                    std::stringstream ss;
                    ss << Utils::replace(joydesc, "button", "");
                    ss >> button;
                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new JoyButtonHandler(joynum, button), key));
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad button " + ss.str());
                    found = true;
                }
                else if (joydesc.find("hat") == 0)
                {
                    Uint8 hat;

                    joydesc = Utils::replace(joydesc, "hat", "");
                    std::stringstream sshat;
                    sshat << joydesc.at(0);
                    int hatnum;
                    sshat >> hatnum;
                    joydesc = joydesc.erase(0, 1);

                    if (joydesc == "leftup") hat = SDL_HAT_LEFTUP;
                    else if (joydesc == "left") hat = SDL_HAT_LEFT;
                    else if (joydesc == "leftdown") hat = SDL_HAT_LEFTDOWN;
                    else if (joydesc == "up") hat = SDL_HAT_UP;
                    //else if(joydesc == "centered") hat = SDL_HAT_CENTERED;
                    else if (joydesc == "down") hat = SDL_HAT_DOWN;
                    else if (joydesc == "rightup") hat = SDL_HAT_RIGHTUP;
                    else if (joydesc == "right") hat = SDL_HAT_RIGHT;
                    else if (joydesc == "rightdown") hat = SDL_HAT_RIGHTDOWN;

                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new JoyHatHandler(joynum, hatnum, hat), key));
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad hat " + joydesc);
                    found = true;
                }
                else if (joydesc.find("axis") == 0)
                {
                    // string is now axis0+
                    unsigned int axis;
                    Sint16       min;
                    Sint16       max;
                    int          deadZone;

                    joydesc = Utils::replace(joydesc, "axis", "");

                    if (!config_.getProperty("controls.deadZone", deadZone))
                    {
                        deadZone = 3;
                    }

                    // string is now 0+
                    if (joydesc.find("-") != std::string::npos)
                    {
                        min = -32768;
                        max = -32768 / 100 * deadZone;
                        joydesc = Utils::replace(joydesc, "-", "");
                    }
                    else if (joydesc.find("+") != std::string::npos)
                    {
                        min = 32767 / 100 * deadZone;
                        max = 32767;
                        joydesc = Utils::replace(joydesc, "+", "");
                    }

                    // string is now just the axis number
                    std::stringstream ss;
                    ss << joydesc;
                    ss >> axis;
                    Logger::write(Logger::ZONE_INFO, "Input", "Binding joypad axis " + ss.str());
                    keyHandlers_.push_back(std::pair<InputHandler *, KeyCode_E>(new JoyAxisHandler(joynum, axis, min, max), key));
                    found = true;
                }*/
	        found = true;
            }

            if (!found)
            {
                Logger::write(Logger::ZONE_ERROR, "Input", "Unsupported property value for " + configKey + "(" + token + "). See Documentation/Keycodes.txt for valid inputs");
                success = false;
            }
        }
    }

    return success;
}

void UserInput::resetStates()
{
    for (unsigned int i = 0; i < keyHandlers_.size(); ++i)
    {
        if (keyHandlers_[i].first)
        {
            keyHandlers_[i].first->reset();
        }
        currentKeyState_[keyHandlers_[i].second] = false;
    }
}


bool UserInput::update( SDL_Event &e )
{
    bool updated = false;

    memcpy( lastKeyState_, currentKeyState_, sizeof( lastKeyState_ ) );
    memset( currentKeyState_, 0, sizeof( currentKeyState_ ) );

    // Handle adding a joystick
    /*if ( e.type == SDL_JOYDEVICEADDED )
    {
        SDL_JoystickID id = SDL_JoystickInstanceID( SDL_JoystickOpen( e.jdevice.which ) );
        for ( unsigned int i = 0; i < cMaxJoy; i++ )
        {
            if ( joysticks_[i] == -1 )
            {
                joysticks_[i] = id;
                break;
            }
        }
    }

    // Handle removing a joystick
    if ( e.type == SDL_JOYDEVICEREMOVED )
    {
        for ( unsigned int i = 0; i < cMaxJoy; i++ )
        {
            if ( joysticks_[i] == e.jdevice.which )
            {
                joysticks_[i] = -1;
                break;
            }
        }
        SDL_JoystickClose( SDL_JoystickFromInstanceID( e.jdevice.which ) );
    }

    // Remap joystick events
    if ( e.type == SDL_JOYAXISMOTION ||
         e.type == SDL_JOYBUTTONUP   ||
         e.type == SDL_JOYBUTTONDOWN ||
         e.type == SDL_JOYHATMOTION )
    {
        for ( unsigned int i = 0; i < cMaxJoy; i++ )
        {
            if ( joysticks_[i] == e.jdevice.which )
            {
                e.jdevice.which = i;
                e.jaxis.which   = i;
                e.jbutton.which = i;
                e.jhat.which    = i;
                break;
            }
        }
    }*/

    for ( unsigned int i = 0; i < keyHandlers_.size( ); ++i )
    {
        InputHandler *h = keyHandlers_[i].first;
        if ( h )
        {
            if ( h->update( e ) ) updated = true;

            currentKeyState_[keyHandlers_[i].second] |= h->pressed( );
        }
    }

    return updated;
}


bool UserInput::keystate(KeyCode_E code)
{
    return currentKeyState_[code];
}

bool UserInput::newKeyPressed(KeyCode_E code)
{
    return currentKeyState_[code] && !lastKeyState_[code];
}

/*
void UserInput::clearJoysticks( )
{
    for ( unsigned int i = 0; i < cMaxJoy; i++ )
    {
        joysticks_[i] = -1;
    }
}*/
