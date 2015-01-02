/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
