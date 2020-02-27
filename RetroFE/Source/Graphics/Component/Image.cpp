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
#include "Image.h"
#include "../ViewInfo.h"
#include "../../SDL.h"
#include "../../Utility/Log.h"
#include <SDL/SDL_image.h>

Image::Image(std::string file, std::string altFile, Page &p, float scaleX, float scaleY, bool dithering)
    : Component(p)
    , texture_(NULL)
    , texture_prescaled_(NULL)
    , ditheringAuthorized_(dithering)
    , needDithering_(false)
    , imgBitsPerPx_(32)
    , file_(file)
    , altFile_(altFile)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
{
    allocateGraphicsMemory();
}

Image::~Image()
{
    freeGraphicsMemory();
}

void Image::freeGraphicsMemory()
{
    Component::freeGraphicsMemory();
    //printf("freeGraphicsMemory: %s\n", file_.c_str());

    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        SDL_FreeSurface(texture_);
        texture_ = NULL;
    }
    if (texture_prescaled_ != NULL)
    {
        SDL_FreeSurface(texture_prescaled_);
	texture_prescaled_ = NULL;
    }
    SDL_UnlockMutex(SDL::getMutex());
}

void Image::allocateGraphicsMemory()
{
    int width;
    int height;

    if(!texture_)
    {
        SDL_LockMutex(SDL::getMutex());

        /* Load image */
        SDL_Surface * img_tmp = NULL;
        //printf("Loading image: %s\n", file_.c_str());
        img_tmp = IMG_Load(file_.c_str());
        if (!img_tmp && altFile_ != "")
        {
	    //printf("	Failed-> Loading backup image: %s\n", altFile_.c_str());
	    img_tmp = IMG_Load(altFile_.c_str());
        }


        if (img_tmp != NULL)
        {
	    /* Check if dithering needed */
	    imgBitsPerPx_ = img_tmp->format->BitsPerPixel;
	    if( imgBitsPerPx_ > 16 && ditheringAuthorized_){
	        needDithering_ = true;
	    }

            /* Convert to RGB 32bit if necessary */
	    if(imgBitsPerPx_ != 32){
	        texture_ = SDL_CreateRGBSurface(0, img_tmp->w, img_tmp->h, 32, 0, 0, 0, 0);
		SDL_BlitSurface(img_tmp, NULL, texture_, NULL);

		/* Free img_tmp */
		SDL_FreeSurface(img_tmp);
	    }
	    else{
	        texture_ = img_tmp;
	    }
	    //SDL_SetAlpha(texture_, SDL_SRCALPHA, 255);

	    /* Set real dimensions */
	    if (texture_ != NULL)
	    {
	      baseViewInfo.ImageWidth = texture_->w * scaleX_;
	      baseViewInfo.ImageHeight = texture_->h * scaleY_;
	    }
        }
        SDL_UnlockMutex(SDL::getMutex());

    }

    Component::allocateGraphicsMemory();

}


void Image::draw()
{
	bool scaling_needed = false;
	bool cache_scaling_needed = false;
	bool use_prescaled = false;

    Component::draw();

    if(texture_)
    {
        SDL_Rect rect;
        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

		/* Cache scaling */
		scaling_needed = rect.w!=0 && rect.h!=0 && (texture_->w != rect.w || texture_->h != rect.h);
		if(scaling_needed){
			cache_scaling_needed = (texture_prescaled_ == NULL)?true:(texture_prescaled_->w != rect.w || texture_prescaled_->h != rect.h);
			if(cache_scaling_needed){
				texture_prescaled_ = SDL::zoomSurface(texture_, NULL, &rect);
				if(texture_prescaled_ == NULL){
					printf("ERROR in %s - Could not create texture_prescaled_\n", __func__);
					use_prescaled = false;
				}
				if(imgBitsPerPx_ > 16 && ditheringAuthorized_){
					needDithering_ = true;
				}
			}

			if(texture_prescaled_ != NULL){
				use_prescaled = true;
			}
		}

		/* Surface to display */
		SDL_Surface * surfaceToRender = NULL;
		if(use_prescaled && texture_prescaled_ != NULL){
			surfaceToRender = texture_prescaled_;
		}
		else{
			surfaceToRender = texture_;
		}

		/* Dithering */
		if(needDithering_){
			//printf("Dither: %s\n", file_.c_str());
			SDL::ditherSurface32bppTo16Bpp(surfaceToRender);
			needDithering_ = false;
		}

		/* Render */
		SDL::renderCopy(surfaceToRender, baseViewInfo.Alpha, NULL, &rect, baseViewInfo);
    }
}
