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

#include "SDL.h"
#include "Database/Configuration.h"
#include "Utility/Log.h"
#include <SDL2/SDL_mixer.h>

SDL_Window *SDL::window_ = NULL;
SDL_Renderer *SDL::renderer_ = NULL;
SDL_mutex *SDL::mutex_ = NULL;
int SDL::displayWidth_ = 0;
int SDL::displayHeight_ = 0;
int SDL::windowWidth_ = 0;
int SDL::windowHeight_ = 0;
bool SDL::fullscreen_ = false;


bool SDL::initialize(Configuration &config)
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

    Logger::write(Logger::ZONE_INFO, "SDL", "Initializing");
    if (retVal && SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::string error = SDL_GetError();
        Logger::write(Logger::ZONE_ERROR, "SDL", "Initialize failed: " + error);
        retVal = false;
    }

    if(retVal && config.getProperty("hideMouse", hideMouse))
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
                displayWidth_ = mode.w;
                displayHeight_ = mode.h;
                break;
            }
        }


        if(!config.getProperty("horizontal", hString))
        {
            Logger::write(Logger::ZONE_ERROR, "Configuration", "Missing property \"horizontal\"");
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
                    windowWidth_ = mode.w;
                    break;
                }
            }
        }
        else if(!config.getProperty("horizontal", windowWidth_))
        {
            Logger::write(Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"horizontal\"");
        }
    }

    if(retVal)
    {
        if(!config.getProperty("vertical", vString))
        {
            Logger::write(Logger::ZONE_ERROR, "Configuration", "Missing property \"vertical\"");
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
                    windowHeight_ = mode.h;
                    break;
                }
            }
        }
        else if(!config.getProperty("vertical", windowHeight_))
        {
            Logger::write(Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"vertical\"");
        }
    }

    if(retVal && !config.getProperty("fullscreen", fullscreen_))
    {
        Logger::write(Logger::ZONE_ERROR, "Configuration", "Missing property: \"fullscreen\"");
        retVal = false;
    }

    if (retVal && fullscreen_)
    {
#ifdef WIN32
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
        windowFlags |= SDL_WINDOW_FULLSCREEN;
#endif
    }

    if(retVal)
    {
        std::string fullscreenStr = fullscreen_ ? "yes" : "no";
        std::stringstream ss;
        ss << "Creating "<< windowWidth_ << "x" << windowHeight_ << " window (fullscreen: " 
           << fullscreenStr << ")";

        Logger::write(Logger::ZONE_INFO, "SDL", ss.str());

        window_ = SDL_CreateWindow("RetroFE",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  windowWidth_,
                                  windowHeight_,
                                  windowFlags);

        if (window_ == NULL)
        {
            std::string error = SDL_GetError();
            Logger::write(Logger::ZONE_ERROR, "SDL", "Create window failed: " + error);
            retVal = false;
        }
    }

    if(retVal)
    {
        renderer_ = SDL_CreateRenderer(window_,
                                      -1,
                                      SDL_RENDERER_ACCELERATED);

        if (renderer_ == NULL)
        {
            std::string error = SDL_GetError();
            Logger::write(Logger::ZONE_ERROR, "SDL", "Create renderer failed: " + error);
            retVal = false;
        }
    }

    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1") != SDL_TRUE)
    {
        Logger::write(Logger::ZONE_ERROR, "SDL", "Improve scale quality. Continuing with low-quality settings.");
    }

    if(retVal)
    {
        mutex_ = SDL_CreateMutex();

        if (mutex_ == NULL)
        {
            std::string error = SDL_GetError();
            Logger::write(Logger::ZONE_ERROR, "SDL", "Mutex creation failed: " + error);
            retVal = false;
        }
    }

    //todo: specify in configuration file
    if (retVal && Mix_OpenAudio(audioRate, audioFormat, audioChannels, audioBuffers) == -1)
    {
        std::string error = Mix_GetError();
        Logger::write(Logger::ZONE_ERROR, "SDL", "Audio initialize failed: " + error);
        retVal = false;
    }

    return retVal;
}

bool SDL::deInitialize()
{
    std::string error = SDL_GetError();
    Logger::write(Logger::ZONE_INFO, "SDL", "DeInitializing");

    Mix_CloseAudio();
    Mix_Quit();
    if(mutex_)
    {
        SDL_DestroyMutex(mutex_);
        mutex_ = NULL;
    }

    if(renderer_)
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = NULL;
    }

    if(window_)
    {
        SDL_DestroyWindow(window_);
        window_ = NULL;
    }

    SDL_ShowCursor(SDL_TRUE);

    SDL_Quit();

    return true;
}

SDL_Renderer* SDL::getRenderer()
{
    return renderer_;
}

SDL_mutex* SDL::getMutex()
{
    return mutex_;
}

SDL_Window* SDL::getWindow()
{
    return window_;
}

bool SDL::renderCopy(SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo)
{

    SDL_Rect srcRect;
    SDL_Rect dstRect;

    SDL_Rect rotateRect;
    rotateRect.w = dest->w;
    rotateRect.h = dest->h;

    if(fullscreen_)
    {
        rotateRect.x = dest->x + (displayWidth_ - windowWidth_)/2;
        rotateRect.y = dest->y + (displayHeight_ - windowHeight_)/2;
    }
    else
    {
        rotateRect.x = dest->x;
        rotateRect.y = dest->y;
    }

    // Create the base fields to check against the container.
    if (src)
    {
        srcRect.x = src->x;
        srcRect.y = src->y;
        srcRect.w = src->w;
        srcRect.h = src->h;
    }
    else
    {
        srcRect.x = 0;
        srcRect.y = 0;
        int w = 0;
        int h = 0;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        srcRect.w = w;
        srcRect.h = h;
    }
    dstRect.x = rotateRect.x;
    dstRect.y = rotateRect.y;
    dstRect.w = rotateRect.w;
    dstRect.h = rotateRect.h;

    // If a container has been defined, limit the display to the container boundaries.
    if (viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0 &&
        rotateRect.w            > 0 && rotateRect.h             > 0)
    {

        // Correct if the image falls to the left of the container
        if (dstRect.x < viewInfo.ContainerX)
        {
            dstRect.x = static_cast<int>(viewInfo.ContainerX);
            srcRect.x = srcRect.x + srcRect.w * (dstRect.x - rotateRect.x) / rotateRect.w;
        }

        // Correct if the image falls to the right of the container
        if (rotateRect.x + rotateRect.w > viewInfo.ContainerX + viewInfo.ContainerWidth)
        {
            dstRect.w = static_cast<int>(viewInfo.ContainerX + viewInfo.ContainerWidth) - dstRect.x;
            srcRect.w = srcRect.w * dstRect.w / rotateRect.w;
        }

        // Correct if the image falls to the top of the container
        if (dstRect.y < viewInfo.ContainerY)
        {
            dstRect.y = static_cast<int>(viewInfo.ContainerY);
            srcRect.y = srcRect.y + srcRect.h * (dstRect.y - rotateRect.y) / rotateRect.h;
        }

        // Correct if the image falls to the bottom of the container
        if (rotateRect.y + rotateRect.h > viewInfo.ContainerY + viewInfo.ContainerHeight)
        {
            dstRect.h = static_cast<int>(viewInfo.ContainerY + viewInfo.ContainerHeight) - dstRect.y;
            srcRect.h = srcRect.h * dstRect.h / rotateRect.h;
        }

    }

    SDL_SetTextureAlphaMod(texture, static_cast<char>(alpha * 255));
    SDL_RenderCopyEx(getRenderer(), texture, &srcRect, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_NONE);

    if (viewInfo.Reflection == "top")
    {
        rotateRect.h = static_cast<unsigned int>(static_cast<float>(rotateRect.h) * viewInfo.ReflectionScale);
        rotateRect.y = rotateRect.y - rotateRect.h - viewInfo.ReflectionDistance;
        SDL_SetTextureAlphaMod(texture, static_cast<char>(viewInfo.ReflectionAlpha * alpha * 255));
        SDL_RenderCopyEx(getRenderer(), texture, src, &rotateRect, viewInfo.Angle, NULL, SDL_FLIP_VERTICAL);
    }

    if (viewInfo.Reflection == "bottom")
    {
        rotateRect.y = rotateRect.y + rotateRect.h + viewInfo.ReflectionDistance;
        rotateRect.h = static_cast<unsigned int>(static_cast<float>(rotateRect.h) * viewInfo.ReflectionScale);
        SDL_SetTextureAlphaMod(texture, static_cast<char>(viewInfo.ReflectionAlpha * alpha * 255));
        SDL_RenderCopyEx(getRenderer(), texture, src, &rotateRect, viewInfo.Angle, NULL, SDL_FLIP_VERTICAL);
    }

    if (viewInfo.Reflection == "left")
    {
        rotateRect.w = static_cast<unsigned int>(static_cast<float>(rotateRect.w) * viewInfo.ReflectionScale);
        rotateRect.x = rotateRect.x - rotateRect.w - viewInfo.ReflectionDistance;
        SDL_SetTextureAlphaMod(texture, static_cast<char>(viewInfo.ReflectionAlpha * alpha * 255));
        SDL_RenderCopyEx(getRenderer(), texture, src, &rotateRect, viewInfo.Angle, NULL, SDL_FLIP_HORIZONTAL);
    }

    if (viewInfo.Reflection == "right")
    {
        rotateRect.x = rotateRect.x + rotateRect.w + viewInfo.ReflectionDistance;
        rotateRect.w = static_cast<unsigned int>(static_cast<float>(rotateRect.w) * viewInfo.ReflectionScale);
        SDL_SetTextureAlphaMod(texture, static_cast<char>(viewInfo.ReflectionAlpha * alpha * 255));
        SDL_RenderCopyEx(getRenderer(), texture, src, &rotateRect, viewInfo.Angle, NULL, SDL_FLIP_HORIZONTAL);
    }

    return true;
}
