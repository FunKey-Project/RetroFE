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
        KeyCodeLetterDown,
        KeyCodeLetterUp,
        KeyCodeAdminMode,
        KeyCodeHideItem,
        KeyCodeQuit
    };

    UserInput(Configuration &c);
    virtual ~UserInput();
    bool initialize();
    SDL_Scancode scancode(KeyCode_E key);
    KeyCode_E keycode(SDL_Scancode scancode);
    bool keystate(SDL_Scancode code, bool state);
    bool keystate(KeyCode_E key);
    bool keyStateChanged();
    void resetKeyStates();



private:
    bool MapKey(std::string keyDescription, KeyCode_E key);
    std::map<KeyCode_E, SDL_Scancode> keyMap_;
    std::map<SDL_Scancode, KeyCode_E> reverseKeyMap_;
    std::map<KeyCode_E, bool> keyState_;
    Configuration &config_;
    const Uint8 *sdlkeys_;
};
