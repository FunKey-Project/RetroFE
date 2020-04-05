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
//#include <SDL/SDL_gfxBlitFunc.h>

/* Basic math */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

/* Get 16bit closest color */
#define CLOSEST_RB(c) (MIN(c+4,0xff) >> 3 << 3)
#define CLOSEST_G(c) (MIN(c+2,0xff) >> 2 << 2)

/* RGB8888 */
#define RGB32BIT(a, r, g, b) ((a<<24) | (r<<16) | (g<<8) | b)
#define GET_A_32BIT(c)  ((c & 0xff000000) >> 24)
#define GET_R_32BIT(c)  ((c & 0x00ff0000) >> 16)
#define GET_G_32BIT(c)  ((c & 0x0000ff00) >> 8)
#define GET_B_32BIT(c)  (c & 0x000000ff)


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

        /*unsigned int rmask;
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
#endif*/
        window_virtual_ = SDL_CreateRGBSurface(0, windowWidth_, windowHeight_, 32, 0,0,0,0);
        //window_virtual_ = SDL_CreateRGBSurface(0, windowWidth_, windowHeight_, 32, rmask, gmask, bmask, amask); // colors are reversed with this !
        if ( window_virtual_ == NULL )
        {
            std::string error = SDL_GetError( );
            Logger::write( Logger::ZONE_ERROR, "SDL", "SDL_CreateRGBSurface window_virtual_ failed: " + error );
            retVal = false;
    }
        SDL_FillRect(window_virtual_, NULL, SDL_MapRGBA(window_virtual_->format, 0, 0, 0, 0));

        /*texture_copy_alpha_ = SDL_CreateRGBSurface(0, windowWidth_, windowHeight_, 32, rmask, gmask, bmask, amask);
        if ( texture_copy_alpha_ == NULL )
        {
	    std::string error = SDL_GetError( );
	    Logger::write( Logger::ZONE_ERROR, "SDL", "SDL_CreateRGBSurface texture_copy_alpha_ failed: " + error );
	    retVal = false;
        }
        SDL_FillRect(texture_copy_alpha_, NULL, SDL_MapRGBA(texture_copy_alpha_->format, 0, 0, 0, 0));*/
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

    while(Mix_Init(0))
        Mix_Quit();

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

    /*if ( texture_copy_alpha_ )
    {
        SDL_FreeSurface(texture_copy_alpha_);
        texture_copy_alpha_ = NULL;
    }*/

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






void SDL::SDL_Rotate_270(SDL_Surface * src, SDL_Surface * dst){
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


/* Dithering 32bpp RGB Surface
 *
 * error diffusion with "Filter Lite" also called "Sierra lite" method
 *
 */
void SDL::ditherSurface32bppTo16Bpp(SDL_Surface * src_surface){

	/* Vars */
	int x, y;
	uint8_t r_old, g_old, b_old;
	uint8_t r_new, g_new, b_new;
	int r_error, g_error, b_error;
	uint32_t cur_px;

	/* Sanity check */
	if(src_surface->format->BitsPerPixel != 32){
		printf("Error: src_surface is %dBpp while dst_surface is not 32\n", src_surface->format->BitsPerPixel);
		return;
	}

	/* Loop for dithering */
	for (y=0; y<src_surface->h; y++){
		for (x=0; x < src_surface->w; x++){

			/* Get old and new rgb values of current pixel */
			cur_px = *((uint32_t*)src_surface->pixels + (y)*src_surface->w + (x));
			r_old = GET_R_32BIT(cur_px);
			g_old = GET_G_32BIT(cur_px);
			b_old = GET_B_32BIT(cur_px);
			r_new = CLOSEST_RB(r_old);
			//g_new = CLOSEST_G(g_old);
			g_new = CLOSEST_RB(g_old);	//RGB555
			b_new = CLOSEST_RB(b_old);

			/* Set new pixel value */
			*((uint32_t*)src_surface->pixels + (y)*src_surface->w + (x)) = RGB32BIT(GET_A_32BIT(cur_px), r_new, g_new, b_new);

			/* Get errors */
			r_error = r_old - r_new;
			g_error = g_old - g_new;
			b_error = b_old - b_new;

			/* Right pixel */
			if(x + 1 < src_surface->w){
				cur_px = *((uint32_t*)src_surface->pixels + (y)*src_surface->w + (x+1));
				r_old = GET_R_32BIT(cur_px);
				g_old = GET_G_32BIT(cur_px);
				b_old = GET_B_32BIT(cur_px);
				*((uint32_t*)src_surface->pixels + (y)*src_surface->w + (x+1)) =
						RGB32BIT( GET_A_32BIT(cur_px),
								MAX(MIN((int)r_old + (r_error>>1), 0xff), 0),
								MAX(MIN((int)g_old + (g_error>>1), 0xff), 0),
								MAX(MIN((int)b_old + (b_error>>1), 0xff), 0) );
			}

			/* Bottom pixel */
			if(y + 1 < src_surface->h){
				cur_px = *((uint32_t*)src_surface->pixels + (y+1)*src_surface->w + (x));
				r_old = GET_R_32BIT(cur_px);
				g_old = GET_G_32BIT(cur_px);
				b_old = GET_B_32BIT(cur_px);
				*((uint32_t*)src_surface->pixels + (y+1)*src_surface->w + (x)) =
						RGB32BIT( GET_A_32BIT(cur_px),
								MAX(MIN((int)r_old + (r_error>>2), 0xff), 0),
								MAX(MIN((int)g_old + (g_error>>2), 0xff), 0),
								MAX(MIN((int)b_old + (b_error>>2), 0xff), 0) );
			}

			/* Bottom left pixel */
			if( x > 0 &&  y + 1 < src_surface->h){
				cur_px = *((uint32_t*)src_surface->pixels + (y+1)*src_surface->w + (x-1));
				r_old = GET_R_32BIT(cur_px);
				g_old = GET_G_32BIT(cur_px);
				b_old = GET_B_32BIT(cur_px);
				*((uint32_t*)src_surface->pixels + (y+1)*src_surface->w + (x-1)) =
						RGB32BIT( GET_A_32BIT(cur_px),
								MAX(MIN((int)r_old + (r_error>>2), 0xff), 0),
								MAX(MIN((int)g_old + (g_error>>2), 0xff), 0),
								MAX(MIN((int)b_old + (b_error>>2), 0xff), 0) );
			}
		}
	}
}


// Copy virtual window to HW window and Flip display
void SDL::renderAndFlipWindow( )
{
	//SDL_BlitSurface(window_virtual_, NULL, window_, NULL);
    memcpy(window_->pixels, window_virtual_->pixels, window_->h*window_->w*sizeof(uint32_t));
	//SDL_Rotate_270(window_virtual_, window_);

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

/// Nearest neighboor optimized with possible out of screen coordinates (for cropping)
SDL_Surface * SDL::zoomSurface(SDL_Surface *src_surface, SDL_Rect *src_rect_origin, SDL_Rect *dst_rect, SDL_Rect *post_cropping_rect){

	/* Declare vars */
	int x_ratio;
	int y_ratio;
	int x2, y2;
	int i, j;
	int rat;
	SDL_Rect srcRect;

	/* Sanity checks */
	if(dst_rect == NULL || src_surface == NULL){
		printf("ERROR in %s, sanity check\n", __func__);
		return NULL;
	}
	if( src_rect_origin == NULL){
	    srcRect.x = 0;
	    srcRect.y = 0;
	    srcRect.w = src_surface->w;
	    srcRect.h = src_surface->h;
	}
	else{
	    srcRect.x = src_rect_origin->x;
	    srcRect.y = src_rect_origin->y;
	    srcRect.w = src_rect_origin->w;
	    srcRect.h = src_rect_origin->h;
	}
	if( srcRect.w > src_surface->w){
		printf("ERROR src_rect->w (%d) > src_surface->w(%d) \n", srcRect.w, src_surface->w);
		srcRect.w = src_surface->w;
	}
	if( srcRect.h > src_surface->h){
		printf("ERROR src_rect->h (%d) > src_surface->h(%d) \n", srcRect.h, src_surface->h);
		srcRect.h = src_surface->h;
	}
	if( srcRect.x > src_surface->w){
		printf("ERROR src_rect->x(%d) > src_rect->w(%d) \n", srcRect.x, srcRect.w);
		return NULL;
	}
	if( srcRect.y > src_surface->h){
		printf("ERROR src_rect->y (%d) > src_rect->h(%d) \n", srcRect.y, srcRect.h);
		return NULL;
	}
	if( post_cropping_rect != NULL ){
		if( post_cropping_rect->w > dst_rect->w){
			post_cropping_rect->w = dst_rect->w;
		}
		if( post_cropping_rect->h > dst_rect->h){
			post_cropping_rect->h = dst_rect->h;
		}
	}

	/*printf("zoomSurface: src_surface->w = %d, src_surface->h = %d, src_surface->BytesPerPixel = %d, src_rect = {%d, %d, %d, %d}, dst_rect =  {%d, %d, %d, %d}\n",
			src_surface->w, src_surface->h, src_surface->format->BytesPerPixel, src_rect->x, src_rect->y, src_rect->w, src_rect->h, dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h);*/

	/* Compute zoom ratio */
	x_ratio = (int)((srcRect.w <<16) / dst_rect->w);
	y_ratio = (int)((srcRect.h <<16) / dst_rect->h);

	/* Create dst surface */
	SDL_Surface *dst_surface = SDL_CreateRGBSurface(src_surface->flags,
			dst_rect->w, dst_rect->h,
			src_surface->format->BitsPerPixel,
			src_surface->format->Rmask, src_surface->format->Gmask,
			src_surface->format->Bmask, src_surface->format->Amask);
	if(dst_surface == NULL){
		printf("ERROR in %s, cannot create dst_surface: %s\n", __func__, SDL_GetError());
	}

	/* Columns iterations */
	for (i = 0; i < dst_surface->h; i++)
	{

		/* Get current lines in src and dst surfaces */
		uint8_t* t = ( (uint8_t*) dst_surface->pixels + (i*dst_surface->w)*dst_surface->format->BytesPerPixel );
		y2 = ((i*y_ratio)>>16);
		uint8_t* p = ( (uint8_t*) src_surface->pixels + ((y2+srcRect.y)*src_surface->w)*src_surface->format->BytesPerPixel );
		rat =  srcRect.x << 16;

		/* Lines iterations */
		for (j = 0; j < dst_surface->w; j++)
		{

			/* Get current pixel in src surface */
			x2 = (rat>>16);

			/* Copy src pixel in dst surface */
			//printf("dst_pix_off = %d, x2=%d, y2=%d, p[%d] = %d\n", t-(uint8_t*)dst_surface->pixels, x2, y2, x2, p[x2*src_surface->format->BytesPerPixel]);
			memcpy(t, p+x2*src_surface->format->BytesPerPixel, dst_surface->format->BytesPerPixel);
			t += dst_surface->format->BytesPerPixel;

			/* Update x position in source surface */
			rat += x_ratio;
		}
	}

	/* Post cropping */
	if(post_cropping_rect != NULL){
		/* Save prev dst_surface ptr */
		SDL_Surface *prev_dst_surface = dst_surface;

		/* Create dst surface */
		dst_surface = SDL_CreateRGBSurface(src_surface->flags,
				post_cropping_rect->w, post_cropping_rect->h,
				src_surface->format->BitsPerPixel,
				src_surface->format->Rmask, src_surface->format->Gmask,
				src_surface->format->Bmask, src_surface->format->Amask);
		if(dst_surface == NULL){
			printf("ERROR in %s, cannot create dst_surface for post cropping: %s\n", __func__, SDL_GetError());
			dst_surface = prev_dst_surface;
		}
		else{
			/*printf("dst_surface is being cropped. prev_dst_surface(%dx%d) -> dst_surface(%dx%d)!!!!!\n",
					prev_dst_surface->w, prev_dst_surface->h, dst_surface->w, dst_surface->h);
			printf("post_cropping_rect [{%d,%d} %dx%d]\n",
					post_cropping_rect->x, post_cropping_rect->y, post_cropping_rect->w, post_cropping_rect->h);*/

			/* Copy cropped surface  */
			if(SDL_BlitSurface(prev_dst_surface, post_cropping_rect, dst_surface, NULL)){
				printf("ERROR in %s, cannot blit previous dst_surface for post cropping: %s\n", __func__, SDL_GetError());
			}

			/* Free previous surface */
			SDL_FreeSurface(prev_dst_surface);
		}
	}

	/* Return new zoomed surface */
	return dst_surface;
}


// Render a copy of a texture
bool SDL::renderCopy( SDL_Surface *texture, float alpha, SDL_Rect *src, SDL_Rect *dest, ViewInfo &viewInfo )
{
	SDL_Surface * surface_to_blit = texture;
	SDL_Surface * texture_zoomed = NULL;
	bool scaling_needed;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
    SDL_Rect srcRectCopy;
    SDL_Rect dstRectCopy;
    double   scaleX;
    double   scaleY;

	/*printf("\nEnter renderCopy -> texture->w = %d, texture->h = %d, src->w = %d, src->h = %d, dest->w = %d, dest->h = %d, viewInfo->ContainerWidth = %d, viewInfo->ContainerHeight = %d\n",
			texture->w, texture->h, src?src->w:4343, src?src->h:4343, dest?dest->w:4242, dest?dest->h:4242, static_cast<int>(viewInfo.ContainerWidth), static_cast<int>(viewInfo.ContainerHeight));
*/

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


	/*printf("before viewinfo remniement -> srcRect->w = %d, srcRect->h = %d, dstRectCopy->w = %d, dstRectCopy->h = %d, viewInfo->ContainerWidth = %f, viewInfo->ContainerHeight = %f\n",
			srcRect.w, srcRect.h, dstRectCopy.w, dstRectCopy.h, viewInfo.ContainerWidth, viewInfo.ContainerHeight);*/

#if 0
    // If a container has been defined, limit the display to the container boundaries.
    if ( viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0 &&
         dstRectCopy.w           > 0 && dstRectCopy.h            > 0 )
    {
        if(viewInfo.ContainerX <= 0)
	    viewInfo.ContainerX = dstRectCopy.x;
	if(viewInfo.ContainerY <= 0)
	    viewInfo.ContainerY = dstRectCopy.y;

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

		/*printf("viewInfo->ContainerWidth = %f, viewInfo->ContainerHeight = %f\n", viewInfo.ContainerWidth, viewInfo.ContainerHeight);
        printf("before viewinfo remaniement -> srcRectCopy = [{%d, %d} %dx%d] ---> srcRect = [{%d, %d} %dx%d]\n",
	srcRectCopy.x, srcRectCopy.y, srcRectCopy.w, srcRectCopy.h, srcRect.x, srcRect.y, srcRect.w, srcRect.h);
	printf("before viewinfo remaniement -> dstRectCopy = [{%d, %d} %dx%d] ---> dstRect = [{%d, %d} %dx%d]\n\n",
	dstRectCopy.x, dstRectCopy.y, dstRectCopy.w, dstRectCopy.h, dstRect.x, dstRect.y, dstRect.w, dstRect.h);*/
    }
#endif

    /* Cropping needed ? */
	bool cropping_needed = false;
	SDL_Rect rect_cropping;
	/*if(viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0)
			printf("Testing if cropping needed ? dst_rect = [{%d, %d} %dx%d]\n",
							dstRect.x, dstRect.y, dstRect.w, dstRect.h);*/
	if ( (viewInfo.ContainerWidth > 0 && viewInfo.ContainerHeight > 0) &&
			(viewInfo.ContainerWidth != dstRect.w || viewInfo.ContainerHeight != dstRect.h) ){
		cropping_needed = true;
		rect_cropping.x = (dstRect.w - viewInfo.ContainerWidth)/2;
		rect_cropping.y = (dstRect.h - viewInfo.ContainerHeight)/2;
		rect_cropping.w = viewInfo.ContainerWidth;
		rect_cropping.h = viewInfo.ContainerHeight;
		dstRect.x += rect_cropping.x;
		dstRect.y += rect_cropping.y;
		/*printf("cropping needed in sdl ?srcRect = [{%d, %d} %dx%d], rect_cropping = [{%d, %d} %dx%d]\n",
				srcRect.x, srcRect.y, srcRect.w, srcRect.h,
				rect_cropping.x, rect_cropping.y, rect_cropping.w, rect_cropping.h);*/
	}

    /* Scaling */
	scaling_needed = (dstRect.w != 0 && dstRect.h!=0) &&
					((!cropping_needed && (srcRect.w != dstRect.w || srcRect.h != dstRect.h)) ||
					(cropping_needed && (srcRect.w != rect_cropping.w || srcRect.h != rect_cropping.h) ));
	if(scaling_needed){
		/*printf("Scaling needed in %s\n", __func__);
		printf("Scaling needed in sdl ?srcRect = [{%d, %d} %dx%d], dst_rect = [{%d, %d} %dx%d]\n",
						srcRect.x, srcRect.y, srcRect.w, srcRect.h,
						dstRect.x, dstRect.y, cropping_needed?rect_cropping.w:dstRect.w, cropping_needed?rect_cropping.h:dstRect.h);*/
		texture_zoomed = zoomSurface(texture, &srcRect, &dstRect, cropping_needed?&rect_cropping:NULL);
		if(texture_zoomed == NULL){
			printf("ERROR in %s - Could not create texture_zoomed\n", __func__);
			return false;
		}
		else{
			surface_to_blit = texture_zoomed;
		}
	}



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

    /*if(alpha){
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
    }*/

    /* Blit surface */
	bool perform_blit = (alpha != 0) && !dstRect.w==0 && !dstRect.h==0;
    if(perform_blit){
        SDL_SetAlpha(surface_to_blit, SDL_SRCALPHA, static_cast<uint8_t>( alpha * 255 ));
        SDL_BlitSurface(surface_to_blit, scaling_needed ? NULL : &srcRect, getWindow(), &dstRect);
    }

    /* Free zoomed texture */
    if(texture_zoomed)
	SDL_FreeSurface(texture_zoomed);






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

