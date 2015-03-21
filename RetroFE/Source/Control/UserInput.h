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
        KeyCodeNull,
        KeyCodeUp,
        KeyCodeDown,
        KeyCodeLeft,
        KeyCodeRight,
        KeyCodeSelect,
        KeyCodeBack,
        KeyCodePageDown,
        KeyCodePageUp,
        KeyCodeAdminMode,
        KeyCodeHideItem,
        KeyCodeQuit
    };

    UserInput(Configuration &c);
    virtual ~UserInput();
    bool Initialize();
    SDL_Scancode GetScancode(KeyCode_E key);
    KeyCode_E GetKeycode(SDL_Scancode scancode);
    bool SetKeyState(SDL_Scancode code, bool state);
    bool GetKeyState(KeyCode_E key);
    bool KeyStateChanged();
    void ResetKeyStates();



private:
    bool MapKey(std::string keyDescription, KeyCode_E key);
    std::map<KeyCode_E, SDL_Scancode> KeyMap;
    std::map<SDL_Scancode, KeyCode_E> ReverseKeyMap;
    std::map<KeyCode_E, bool> KeyState;
    Configuration &Config;
    const Uint8 *SDLKeys;
};
