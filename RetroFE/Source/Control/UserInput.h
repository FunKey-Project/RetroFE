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
#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <map>
#include <string>
#include <vector>

class Configuration;
class InputHandler;

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
        KeyCodeFavPlaylist,
        KeyCodeNextPlaylist,
        KeyCodePrevPlaylist,
        KeyCodeRandom,
        KeyCodeAddPlaylist,
        KeyCodeRemovePlaylist,
        KeyCodeAdminMode,
        KeyCodeHideItem,
        KeyCodeQuit,
        KeyCodeMax
    };

    UserInput(Configuration &c);
    virtual ~UserInput();
    bool initialize();
    void resetStates();
    bool update(SDL_Event &e);
    bool keystate(KeyCode_E);
    bool newKeyPressed(KeyCode_E code);

private:
    bool MapKey(std::string keyDescription, KeyCode_E key);
    bool MapKey(std::string keyDescription, KeyCode_E key, bool required);
    Configuration &config_;
    std::vector<SDL_Joystick *> joysticks_;
    std::vector<std::pair<InputHandler *, KeyCode_E>> keyHandlers_;
    bool lastKeyState_[KeyCodeMax]; 
    bool currentKeyState_[KeyCodeMax]; 
};
