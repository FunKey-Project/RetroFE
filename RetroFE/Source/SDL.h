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


//#include <SDL/SDL.h>
#include <SDL/SDL.h>
#include <string>
#include "Graphics/ViewInfo.h"

//Flip flags
#define FLIP_VERTICAL	1
#define FLIP_HORIZONTAL	2

class Configuration;


class SDL
{
public:
    static bool initialize( Configuration &config );
    static bool deInitialize( );
    //static SDL_Renderer *getRenderer( );
    static SDL_mutex *getMutex( );
    //static SDL_Window *getWindow( );
    static SDL_Surface *getWindow( );
    static void renderAndFlipWindow( );
    //static bool renderCopy( SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo );
    static bool renderCopy( SDL_Surface *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo );
    static int getWindowWidth( )
    {
        return windowWidth_;
    }
    static int getWindowHeight( )
    {
        return windowHeight_;
    }
    static bool isFullscreen( )
    {
        return fullscreen_;
    }
    static void SDL_Rotate_270(SDL_Surface * dst, SDL_Surface * src);

private:
    //static SDL_Window   *window_;
    //static SDL_Renderer *renderer_;
    static Uint32 get_pixel32( SDL_Surface *surface, int x, int y );
    static void put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel );
    static SDL_Surface * flip_surface( SDL_Surface *surface, int flags );
    static SDL_Surface  *window_;
    static SDL_Surface 	*window_virtual_;
    static SDL_Surface 	*texture_copy_alpha_;
    static SDL_mutex    *mutex_;
    static int           displayWidth_;
    static int           displayHeight_;
    static int           windowWidth_;
    static int           windowHeight_;
    static bool          fullscreen_;
    static bool          showFrame_;
};

