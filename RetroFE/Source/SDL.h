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

// todo: this wrapper could be cleaned up
class Configuration;

class SDL
{
public:
    static bool Initialize(Configuration &config);
    static bool DeInitialize();
    static SDL_Renderer *GetRenderer();
    static SDL_mutex *GetMutex();
    static SDL_Window *GetWindow();
    static bool RenderCopy(SDL_Texture *texture, unsigned char alpha, SDL_Rect *src, SDL_Rect *dest, double angle);
    static int GetWindowWidth()
    {
        return WindowWidth;
    }
    static int GetWindowHeight()
    {
        return WindowHeight;
    }
    static bool IsFullscreen()
    {
        return Fullscreen;
    }

private:
    static SDL_Window *Window;
    static SDL_Renderer *Renderer;
    static SDL_mutex *Mutex;
    static int DisplayWidth;
    static int DisplayHeight;
    static int WindowWidth;
    static int WindowHeight;
    static bool Fullscreen;
};
