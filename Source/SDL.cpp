/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "SDL.h"
#include "Database/Configuration.h"
#include "Utility/Log.h"
#include <SDL2/SDL_mixer.h>

SDL_Window *SDL::Window = NULL;
SDL_Renderer *SDL::Renderer = NULL;
SDL_mutex *SDL::Mutex = NULL;
int SDL::DisplayWidth = 0;
int SDL::DisplayHeight = 0;
int SDL::WindowWidth = 0;
int SDL::WindowHeight = 0;
bool SDL::Fullscreen = false;


bool SDL::Initialize(Configuration &config)
{
    bool retVal = true;
    std::string hString;
    std::string vString;
    Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS;
    int audioRate = MIX_DEFAULT_FREQUENCY;
    Uint16 audioFormat = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
    int audioChannels = 1;
    int audioBuffers = 4096;
    bool hideMouse;

    Logger::Write(Logger::ZONE_DEBUG, "SDL", "Initializing");
    if (retVal && SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::string error = SDL_GetError();
        Logger::Write(Logger::ZONE_ERROR, "SDL", "Initialize failed: " + error);
        retVal = false;
    }

    if(retVal && config.GetProperty("hideMouse", hideMouse))
    {
        if(hideMouse)
        {
            SDL_ShowCursor(SDL_FALSE);
        }
        else
        {
            SDL_ShowCursor(SDL_TRUE);
        }
    }

    // check for a few other necessary Configurations
    if(retVal)
    {
        // Get current display mode of all displays.
        for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
        {
            SDL_DisplayMode mode;
            if(SDL_GetCurrentDisplayMode(i, &mode) == 0)
            {
                DisplayWidth = mode.w;
                DisplayHeight = mode.h;
                break;
            }
        }


        if(!config.GetProperty("horizontal", hString))
        {
            Logger::Write(Logger::ZONE_ERROR, "Configuration", "Missing property \"horizontal\"");
            retVal = false;
        }
        else if(hString == "stretch")
        {
            // Get current display mode of all displays.
            for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
            {
                SDL_DisplayMode mode;
                if(SDL_GetCurrentDisplayMode(i, &mode) == 0)
                {
                    WindowWidth = mode.w;
                    break;
                }
            }
        }
        else if(!config.GetProperty("horizontal", WindowWidth))
        {
            Logger::Write(Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"horizontal\"");
        }
    }

    if(retVal)
    {
        if(!config.GetProperty("vertical", vString))
        {
            Logger::Write(Logger::ZONE_ERROR, "Configuration", "Missing property \"vertical\"");
            retVal = false;
        }
        else if(vString == "stretch")
        {
            // Get current display mode of all displays.
            for(int i = 0; i < SDL_GetNumVideoDisplays(); ++i)
            {
                SDL_DisplayMode mode;
                if(SDL_GetDesktopDisplayMode(i, &mode) == 0)
                {
                    WindowHeight = mode.h;
                    break;
                }
            }
        }
        else if(!config.GetProperty("vertical", WindowHeight))
        {
            Logger::Write(Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"vertical\"");
        }
    }

    if(retVal && !config.GetProperty("fullscreen", Fullscreen))
    {
        Logger::Write(Logger::ZONE_ERROR, "Configuration", "Missing property: \"fullscreen\"");
        retVal = false;
    }

    if (retVal && Fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    if(retVal)
    {
        std::stringstream ss;
        ss << "Creating "<< WindowWidth << "x" << WindowHeight << " window (fullscreen: " << Fullscreen << ")";
        Logger::Write(Logger::ZONE_DEBUG, "SDL", ss.str());

        Window = SDL_CreateWindow("RetroFE",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  WindowWidth,
                                  WindowHeight,
                                  windowFlags);

        if (Window == NULL)
        {
            std::string error = SDL_GetError();
            Logger::Write(Logger::ZONE_ERROR, "SDL", "Create window failed: " + error);
            retVal = false;
        }
    }

    if(retVal)
    {
        Renderer = SDL_CreateRenderer(Window,
                                      -1,
                                      SDL_RENDERER_ACCELERATED);

        if (Renderer == NULL)
        {
            std::string error = SDL_GetError();
            Logger::Write(Logger::ZONE_ERROR, "SDL", "Create renderer failed: " + error);
            retVal = false;
        }
    }

    if(retVal)
    {
        Mutex = SDL_CreateMutex();

        if (Mutex == NULL)
        {
            std::string error = SDL_GetError();
            Logger::Write(Logger::ZONE_ERROR, "SDL", "Mutex creation failed: " + error);
            retVal = false;
        }
    }

    //todo: specify in configuration file
    if (retVal && Mix_OpenAudio(audioRate, audioFormat, audioChannels, audioBuffers) == -1)
    {
        std::string error = Mix_GetError();
        Logger::Write(Logger::ZONE_ERROR, "SDL", "Audio initialize failed: " + error);
        retVal = false;
    }

    return retVal;
}

bool SDL::DeInitialize()
{
    std::string error = SDL_GetError();
    Logger::Write(Logger::ZONE_DEBUG, "SDL", "DeInitializing");

    Mix_CloseAudio();
    Mix_Quit();
    if(Mutex)
    {
        SDL_DestroyMutex(Mutex);
        Mutex = NULL;
    }

    if(Renderer)
    {
        SDL_DestroyRenderer(Renderer);
        Renderer = NULL;
    }

    if(Window)
    {
        SDL_DestroyWindow(Window);
        Window = NULL;
    }

    SDL_ShowCursor(SDL_TRUE);

    SDL_Quit();

    return true;
}

SDL_Renderer* SDL::GetRenderer()
{
    return Renderer;
}

SDL_mutex* SDL::GetMutex()
{
    return Mutex;
}

SDL_Window* SDL::GetWindow()
{
    return Window;
}


bool SDL::RenderCopy(SDL_Texture *texture, unsigned char transparency, SDL_Rect *src, SDL_Rect *dest, double angle)
{
    SDL_Rect rotateRect;
    rotateRect.w = dest->w;
    rotateRect.h = dest->h;

    if(Fullscreen)
    {
        rotateRect.x = dest->x + (DisplayWidth - WindowWidth)/2;
        rotateRect.y = dest->y + (DisplayHeight - WindowHeight)/2;
    }
    else
    {
        rotateRect.x = dest->x;
        rotateRect.y = dest->y;
    }

    SDL_SetTextureAlphaMod(texture, transparency);
    SDL_RenderCopyEx(GetRenderer(), texture, src, &rotateRect, angle, NULL, SDL_FLIP_NONE);

    return true;
}
