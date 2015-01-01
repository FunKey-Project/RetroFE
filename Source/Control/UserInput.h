/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once
#include <map>
#include <SDL2/SDL.h>
#include <string>

class Configuration;

class UserInput
{
public:
    enum KeyCode_E
    {
        KeyCodeNextItem,
        KeyCodePreviousItem,
        KeyCodeSelect,
        KeyCodeBack,
        KeyCodePageDown,
        KeyCodePageUp,
        KeyCodeAdminMode,
        KeyCodeHideItem,
        KeyCodeQuit
    };

    UserInput(Configuration *c);
    virtual ~UserInput();
    bool Initialize();
    SDL_Scancode GetScancode(KeyCode_E key);


private:
    bool MapKey(std::string keyDescription, KeyCode_E key);
    std::map<KeyCode_E, SDL_Scancode> KeyMap;

    Configuration *Config;
};
