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
#include <SDL/SDL_mixer.h>
//#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_gfxBlitFunc.h>


//SDL_Window   *SDL::window_        = NULL;
//SDL_Renderer *SDL::renderer_      = NULL;
SDL_Surface  *SDL::window_		  		= NULL;
SDL_Surface  *SDL::window_virtual_		= NULL;
SDL_Surface  *SDL::texture_copy_alpha_	= NULL;
SDL_mutex    *SDL::mutex_         = NULL;
int           SDL::displayWidth_  = 0;
int           SDL::displayHeight_ = 0;
int           SDL::windowWidth_   = 0;
int           SDL::windowHeight_  = 0;
bool          SDL::fullscreen_    = false;
bool          SDL::showFrame_    		= true;


// Initialize SDL
bool SDL::initialize( Configuration &config )
{

    bool        retVal        = true;
    std::string hString;
    std::string vString;
    Uint32      windowFlags   = SDL_HWSURFACE | SDL_DOUBLEBUF;
    int         audioRate     = MIX_DEFAULT_FREQUENCY;
    Uint16      audioFormat   = MIX_DEFAULT_FORMAT; /* 16-bit stereo */
    int         audioChannels = 1;
    int         audioBuffers  = 4096;
    bool        hideMouse;
    const SDL_VideoInfo* videoInfo;

    Logger::write( Logger::ZONE_INFO, "SDL", "Initializing" );
    if (retVal && SDL_Init( SDL_INIT_EVERYTHING ) != 0)
    {
        std::string error = SDL_GetError( );
        Logger::write( Logger::ZONE_ERROR, "SDL", "Initialize failed: " + error );
        retVal = false;
    }

    if ( retVal && config.getProperty( "hideMouse", hideMouse ) )
    {
        if ( hideMouse )
        {
            SDL_ShowCursor( SDL_FALSE );
        }
        else
        {
            SDL_ShowCursor( SDL_TRUE );
        }
    }

    // check for a few other necessary Configurations
    if ( retVal )
    {
        // Get current screen resolution
        videoInfo = SDL_GetVideoInfo();
	if (videoInfo == NULL)
        {
	    Logger::write( Logger::ZONE_ERROR, "SDL", "SDL_GetVideoInfo failed");
	    retVal = false;
	}
	else{
	    displayWidth_ = videoInfo->current_w;
	    displayHeight_ = videoInfo->current_h;
        }

        if ( !config.getProperty( "horizontal", hString ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property \"horizontal\"" );
            retVal = false;
        }
        else if ( hString == "stretch" )
        {
	    windowWidth_ = displayWidth_;
        }
        else if ( !config.getProperty( "horizontal", windowWidth_ ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"horizontal\"" );
        }
    }

    if ( retVal )
    {
        if ( !config.getProperty( "vertical", vString ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property \"vertical\"" );
            retVal = false;
        }
        else if ( vString == "stretch" )
        {
	    windowHeight_ = displayHeight_;
        }
        else if ( !config.getProperty( "vertical", windowHeight_ ) )
        {
            Logger::write( Logger::ZONE_ERROR, "Configuration", "Invalid property value for \"vertical\"" );
        }
    }

    if ( retVal && !config.getProperty( "fullscreen", fullscreen_ ) )
    {
        Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property: \"fullscreen\"" );
        retVal = false;
    }

    if (retVal && fullscreen_)
    {
        windowFlags |= SDL_FULLSCREEN;
    }

    if ( retVal && !config.getProperty( "showFrame", showFrame_ ) )
    {
        Logger::write( Logger::ZONE_ERROR, "Configuration", "Missing property: \"showFrame\"" );
        retVal = false;
    }

    if (retVal && !showFrame_)
    {
        windowFlags |= SDL_NOFRAME;
    }

    if ( retVal )
    {
        std::string fullscreenStr = fullscreen_ ? "yes" : "no";
        std::stringstream ss;
        ss << "Creating "<< windowWidth_ << "x" << windowHeight_ << " window (fullscreen: " << fullscreenStr << ")";

        Logger::write( Logger::ZONE_INFO, "SDL", ss.str( ));

        window_ = SDL_SetVideoMode(windowWidth_, windowHeight_, 32, windowFlags);
        if ( window_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "SDL_SetVideoMode failed: " + error );
            retVal = false;
        }

        unsigned int rmask;
        unsigned int gmask;
        unsigned int bmask;
        unsigned int amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif
        window_virtual_ = SDL_CreateRGBSurface(0, windowWidth_, windowHeight_, 32, 0,0,0,0);
        //window_virtual_ = SDL_CreateRGBSurface(0, windowWidth_, windowHeight_, 32, rmask, gmask, bmask, amask); // colors are reversed with this !
        if ( window_virtual_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "SDL_CreateRGBSurface window_virtual_ failed: " + error );
            retVal = false;
    }
        SDL_FillRect(window_virtual_, NULL, SDL_MapRGBA(window_virtual_->format, 0, 0, 0, 0));

        texture_copy_alpha_ = SDL_CreateRGBSurface(0, windowWidth_, windowHeight_, 32, rmask, gmask, bmask, amask);
        if ( texture_copy_alpha_ == NULL )
        {
	    std::string error = SDL_GetError( );
	    Logger::write( Logger::ZONE_ERROR, "SDL", "SDL_CreateRGBSurface texture_copy_alpha_ failed: " + error );
	    retVal = false;
        }
        SDL_FillRect(texture_copy_alpha_, NULL, SDL_MapRGBA(texture_copy_alpha_->format, 0, 0, 0, 0));
    }
    /*if ( retVal )
    {
        renderer_ = SDL_CreateRenderer( window_, -1, SDL_RENDERER_ACCELERATED );

        if ( renderer_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "Create renderer failed: " + error );
            retVal = false;
        }
    }*/

    /*if ( SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1") != SDL_TRUE )
    {
        Logger::write( Logger::ZONE_ERROR, "SDL", "Improve scale quality. Continuing with low-quality settings." );
    }*/

    /*bool minimize_on_focus_loss_;
    if ( config.getProperty( "minimize_on_focus_loss", minimize_on_focus_loss_ ) )
    {
        if ( minimize_on_focus_loss_ )
        {
            SDL_SetHintWithPriority( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1", SDL_HINT_OVERRIDE );
        }
        else
        {
            SDL_SetHintWithPriority( SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0", SDL_HINT_OVERRIDE );
        }
    }*/

    if ( retVal )
    {
        mutex_ = SDL_CreateMutex( );

        if ( mutex_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "Mutex creation failed: " + error );
            retVal = false;
        }
    }

    if ( retVal && Mix_OpenAudio( audioRate, audioFormat, audioChannels, audioBuffers ) == -1 )
    {
        std::string error = Mix_GetError( );
        Logger::write( Logger::ZONE_WARNING, "SDL", "Audio initialize failed: " + error );
    }

    return retVal;

}


// Deinitialize SDL
bool SDL::deInitialize( )
{
    std::string error = SDL_GetError( );
    Logger::write( Logger::ZONE_INFO, "SDL", "DeInitializing" );

    Mix_CloseAudio( );
    Mix_Quit( );

    if ( mutex_ )
    {
        SDL_DestroyMutex(mutex_);
        mutex_ = NULL;
    }

    /*if ( renderer_ )
    {
        SDL_DestroyRenderer(renderer_);
        renderer_ = NULL;
    }*/

    if ( window_virtual_ )
    {
        SDL_FreeSurface(window_virtual_);
        window_virtual_ = NULL;
    }

    if ( texture_copy_alpha_ )
    {
        SDL_FreeSurface(texture_copy_alpha_);
        texture_copy_alpha_ = NULL;
    }

    SDL_ShowCursor( SDL_TRUE );

    SDL_Quit( );

    return true;
}


// Get the renderer
/*SDL_Renderer* SDL::getRenderer( )
{
    return renderer_;
}*/


// Get the mutex
SDL_mutex* SDL::getMutex( )
{
    return mutex_;
}


// Get the window
SDL_Surface* SDL::getWindow( )
{
    //return window_;
	return window_virtual_;
}
/*SDL_Window* SDL::getWindow( )
{
    return window_;
}*/






void SDL::SDL_Rotate_270(SDL_Surface * dst, SDL_Surface * src){
  int i, j;
    uint32_t *source_pixels = (uint32_t*) src->pixels;
    uint32_t *dest_pixels = (uint32_t*) dst->pixels;

    /// --- Checking for right pixel format ---
    //MENU_DEBUG_PRINTF("Source bpb = %d, Dest bpb = %d\n", src->format->BitsPerPixel, dst->format->BitsPerPixel);
    if(src->format->BitsPerPixel != 32){
      printf("Error in SDL_Rotate_270, Wrong virtual_hw_surface pixel format: %d bpb, expected: uint32_t bpb\n", src->format->BitsPerPixel);
      return;
}
    if(dst->format->BitsPerPixel != 32){
      printf("Error in SDL_Rotate_270, Wrong hw_surface pixel format: %d bpb, expected: uint32_t bpb\n", dst->format->BitsPerPixel);
      return;
    }

    /// --- Checking if same dimensions ---
    if(dst->w != src->w || dst->h != src->h){
      printf("Error in SDL_Rotate_270, hw_surface (%dx%d) and virtual_hw_surface (%dx%d) have different dimensions\n",
	     dst->w, dst->h, src->w, src->h);
      return;
    }

  /// --- Pixel copy and rotation (270) ---
  uint32_t *cur_p_src, *cur_p_dst;
  for(i=0; i<src->h; i++){
    for(j=0; j<src->w; j++){
      cur_p_src = source_pixels + i*src->w + j;
      cur_p_dst = dest_pixels + (dst->h-1-j)*dst->w + i;
      *cur_p_dst = *cur_p_src;
    }
  }
}


// Copy virtual window to HW window and Flip display
void SDL::renderAndFlipWindow( )
{
	//SDL_BlitSurface(window_virtual_, NULL, window_, NULL);
	SDL_Rotate_270(window_, window_virtual_);

	SDL_Flip(window_);
}



Uint32 SDL::get_pixel32( SDL_Surface *surface, int x, int y )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;

    //Get the requested pixel
    return pixels[ ( y * surface->w ) + x ];
}

void SDL::put_pixel32( SDL_Surface *surface, int x, int y, Uint32 pixel )
{
    //Convert the pixels to 32 bit
    Uint32 *pixels = (Uint32 *)surface->pixels;

    //Set the pixel
    pixels[ ( y * surface->w ) + x ] = pixel;
}


// Flip a surface
SDL_Surface * SDL::flip_surface( SDL_Surface *surface, int flags )
{
    //Pointer to the soon to be flipped surface
    SDL_Surface *flipped = NULL;

    //If the image is color keyed
    if( surface->flags & SDL_SRCCOLORKEY )
    {
        flipped = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, 0 );
    }
    //Otherwise
    else
    {
        flipped = SDL_CreateRGBSurface( SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask );
    }

    //If the surface must be locked
    if( SDL_MUSTLOCK( surface ) )
    {
        //Lock the surface
        SDL_LockSurface( surface );
    }

    //Go through columns
    for( int x = 0, rx = flipped->w - 1; x < flipped->w; x++, rx-- )
    {
        //Go through rows
        for( int y = 0, ry = flipped->h - 1; y < flipped->h; y++, ry-- )
        {
            //Get pixel
            Uint32 pixel = get_pixel32( surface, x, y );

            //Copy pixel
            if( ( flags & FLIP_VERTICAL ) && ( flags & FLIP_HORIZONTAL ) )
            {
                put_pixel32( flipped, rx, ry, pixel );
            }
            else if( flags & FLIP_HORIZONTAL )
            {
                put_pixel32( flipped, rx, y, pixel );
            }
            else if( flags & FLIP_VERTICAL )
            {
                put_pixel32( flipped, x, ry, pixel );
            }
        }
    }

    //Unlock surface
    if( SDL_MUSTLOCK( surface ) )
    {
        SDL_UnlockSurface( surface );
    }

    //Copy color key
    if( surface->flags & SDL_SRCCOLORKEY )
    {
        SDL_SetColorKey( flipped, SDL_RLEACCEL | SDL_SRCCOLORKEY, surface->format->colorkey );
    }

    //Return flipped surface
    return flipped;
}





// Render a copy of a texture
/*bool SDL::renderCopy( SDL_Texture *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo )*/
bool SDL::renderCopy( SDL_Surface *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo )
{

    SDL_Rect srcRect;
    SDL_Rect dstRect;
    SDL_Rect srcRectCopy;
    SDL_Rect dstRectCopy;
    double   scaleX;
    double   scaleY;

    dstRect.w = dest->w;
    dstRect.h = dest->h;

    if ( fullscreen_ )
    {
        dstRect.x = dest->x + (displayWidth_ - windowWidth_)/2;
        dstRect.y = dest->y + (displayHeight_ - windowHeight_)/2;
    }
    else
    {
        dstRect.x = dest->x;
        dstRect.y = dest->y;
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
        /*int w = 0;
        int h = 0;
        SDL_QueryTexture(texture, NULL, NULL, &w, &h);
        srcRect.w = w;
        srcRect.h = h;*/
        srcRect.w = texture->w;
        srcRect.h = texture->h;
    }

    // Define the scale
    scaleX = (dstRect.w > 0) ? static_cast<double>( srcRect.w ) / static_cast<double>( dstRect.w ) : 0.0;
    scaleY = (dstRect.h > 0) ? static_cast<double>( srcRect.h ) / static_cast<double>( dstRect.h ) : 0.0;

    // Make a copy
    srcRectCopy.x = srcRect.x;
    srcRectCopy.y = srcRect.y;
    srcRectCopy.w = srcRect.w;
    srcRectCopy.h = srcRect.h;
    dstRectCopy.x = dstRect.x;
    dstRectCopy.y = dstRect.y;
    dstRectCopy.w = dstRect.w;
    dstRectCopy.h = dstRect.h;

    // If a container has been defined, limit the display to the container boundaries.
    if ( viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0 &&
         dstRectCopy.w           > 0 && dstRectCopy.h            > 0 )
    {

        // Correct if the image falls to the left of the container
        if ( dstRect.x < viewInfo.ContainerX )
        {
            dstRect.x = static_cast<int>( viewInfo.ContainerX );
            dstRect.w = dstRectCopy.w + dstRectCopy.x - dstRect.x;
            srcRect.x = srcRectCopy.x + srcRectCopy.w * (dstRect.x - dstRectCopy.x) / dstRectCopy.w;
        }

        // Correct if the image falls to the right of the container
        if ( (dstRectCopy.x + dstRectCopy.w) > (viewInfo.ContainerX + viewInfo.ContainerWidth) )
        {
            dstRect.w = static_cast<int>( viewInfo.ContainerX + viewInfo.ContainerWidth ) - dstRect.x;
        }

        // Correct if the image falls to the top of the container
        if ( dstRect.y < viewInfo.ContainerY )
        {
            dstRect.y = static_cast<int>( viewInfo.ContainerY );
            dstRect.h = dstRectCopy.h + dstRectCopy.y - dstRect.y;
            srcRect.y = srcRectCopy.y + srcRectCopy.h * (dstRect.y - dstRectCopy.y) / dstRectCopy.h;
        }

        // Correct if the image falls to the bottom of the container
        if ( (dstRectCopy.y + dstRectCopy.h) > (viewInfo.ContainerY + viewInfo.ContainerHeight) )
        {
            dstRect.h = static_cast<int>( viewInfo.ContainerY + viewInfo.ContainerHeight ) - dstRect.y;
        }

        // Define source width and height
        srcRect.w = static_cast<int>( dstRect.w * scaleX );
        srcRect.h = static_cast<int>( dstRect.h * scaleY );

    }

    //printf("scaleX : %d, scale Y:%d\n", scaleX, scaleY);
    //SDL_SetTextureAlphaMod( texture, static_cast<char>( alpha * 255 ) );
    //SDL_RenderCopyEx( getRenderer( ), texture, &srcRect, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_NONE );


    /*if(texture->format->Amask){
		SDL_SetAlpha( texture, 0, SDL_ALPHA_OPAQUE );
		SDL_BlitSurface (texture, &srcRect, texture_copy_alpha_, &dstRect);
		SDL_gfxMultiplyAlpha (texture_copy_alpha_, static_cast<char>( alpha * 255 ));
		SDL_BlitSurface (texture_copy_alpha_, &dstRect, getWindow(), &dstRect);
    }
    else{
        //SDL_SetAlpha(texture, SDL_SRCALPHA, static_cast<char>( alpha * 255 ));
	SDL_gfxSetAlpha(texture, static_cast<char>( alpha * 255 ));
	SDL_BlitSurface (texture, &srcRect, getWindow(), &dstRect);
    }*/

    /*SDL_SetAlpha(texture, SDL_SRCALPHA, static_cast<char>( alpha * 255 ));
    SDL_BlitSurface (texture, &srcRect, getWindow(), &dstRect);*/


    unsigned int rmask;
    unsigned int gmask;
    unsigned int bmask;
    unsigned int amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif
    SDL_Surface * texture_tmp = SDL_CreateRGBSurface(0, texture->w, texture->h, 32, rmask, gmask, bmask, amask);
    //SDL_FillRect(texture_tmp, NULL, SDL_MapRGBA(texture_tmp->format, 0, 0, 0, 0));
    SDL_SetAlpha( texture, 0, SDL_ALPHA_OPAQUE );
    SDL_BlitSurface (texture, NULL, texture_tmp, NULL);
    SDL_gfxMultiplyAlpha (texture_tmp, static_cast<char>( alpha * 255 ));
    //SDL_gfxBlitRGBA(texture_tmp, &srcRect, getWindow(), &dstRect);
    SDL_BlitSurface(texture_tmp, &srcRect, getWindow(), &dstRect);
    SDL_FreeSurface(texture_tmp);


    //texture = rotozoomSurfaceXY(texture, viewInfo.Angle, scaleX, scaleY, SMOOTHING_OFF);
    //texture = rotozoomSurfaceXY(texture, viewInfo.Angle, 0.5, 0.5, SMOOTHING_OFF);
    //double scale_x = 240/1920;
    //double scale_y = 240/1080;
    //texture = rotozoomSurfaceXY(texture, 0, 0.5, 0.6, SMOOTHING_ON);

    /*if ( viewInfo.Reflection == "top" )
    {
        dstRect.h = static_cast<unsigned int>( static_cast<float>(dstRect.h ) * viewInfo.ReflectionScale);
        dstRect.y = dstRect.y - dstRect.h - viewInfo.ReflectionDistance;
        //SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        //SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_VERTICAL );

        //texture = rotozoomSurfaceXY(texture, viewInfo.Angle, scaleX, scaleY, SMOOTHING_ON);


        SDL_SetAlpha( texture, 0, SDL_ALPHA_OPAQUE );
        SDL_BlitSurface (texture, &srcRect, texture_copy_alpha_, &dstRect);
        texture_copy_alpha_ = flip_surface(texture_copy_alpha_, FLIP_VERTICAL);
        SDL_gfxMultiplyAlpha (texture_copy_alpha_, static_cast<char>( alpha * 255 ));
        SDL_BlitSurface (texture_copy_alpha_, &dstRect, getWindow(), &dstRect);
    }*/

    /*if ( viewInfo.Reflection == "bottom" )
    {
        dstRect.y = dstRect.y + dstRect.h + viewInfo.ReflectionDistance;
        dstRect.h = static_cast<unsigned int>( static_cast<float>(dstRect.h ) * viewInfo.ReflectionScale);
        //SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        //SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_VERTICAL );
        SDL_SetAlpha(texture, SDL_SRCALPHA, static_cast<char>( alpha * 255 ));
        //texture = flip_surface(texture, FLIP_VERTICAL);
        //texture = rotozoomSurfaceXY(texture, viewInfo.Angle, scaleX, scaleY, SMOOTHING_ON);
        SDL_BlitSurface(texture, src, getWindow(), &dstRect);
    }

    if ( viewInfo.Reflection == "left" )
    {
        dstRect.w = static_cast<unsigned int>( static_cast<float>(dstRect.w ) * viewInfo.ReflectionScale);
        dstRect.x = dstRect.x - dstRect.w - viewInfo.ReflectionDistance;
        //SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        //SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_HORIZONTAL );
        SDL_SetAlpha(texture, SDL_SRCALPHA, static_cast<char>( alpha * 255 ));
        //texture = flip_surface(texture, FLIP_HORIZONTAL);
        //texture = rotozoomSurfaceXY(texture, viewInfo.Angle, scaleX, scaleY, SMOOTHING_ON);
        SDL_BlitSurface(texture, src, getWindow(), &dstRect);
    }

    if ( viewInfo.Reflection == "right" )
    {
        dstRect.x = dstRect.x + dstRect.w + viewInfo.ReflectionDistance;
        dstRect.w = static_cast<unsigned int>( static_cast<float>(dstRect.w ) * viewInfo.ReflectionScale);
        //SDL_SetTextureAlphaMod( texture, static_cast<char>( viewInfo.ReflectionAlpha * alpha * 255 ) );
        //SDL_RenderCopyEx( getRenderer( ), texture, src, &dstRect, viewInfo.Angle, NULL, SDL_FLIP_HORIZONTAL );
        SDL_SetAlpha(texture, SDL_SRCALPHA, static_cast<char>( alpha * 255 ));
        //texture = flip_surface(texture, FLIP_HORIZONTAL);
        //texture = rotozoomSurfaceXY(texture, viewInfo.Angle, scaleX, scaleY, SMOOTHING_ON);
        SDL_BlitSurface(texture, src, getWindow(), &dstRect);
    }*/

    return true;
}

