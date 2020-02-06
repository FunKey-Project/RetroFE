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

Image::Image(std::string file, std::string altFile, Page &p, float scaleX, float scaleY)
    : Component(p)
    , texture_(NULL)
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

    SDL_LockMutex(SDL::getMutex());
    if (texture_ != NULL)
    {
        //SDL_DestroyTexture(texture_);
	SDL_FreeSurface(texture_);
        texture_ = NULL;
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
        //texture_ = IMG_LoadTexture(SDL::getRenderer(), file_.c_str());
        //texture_ = IMG_Load(file_.c_str());
        SDL_Surface *img_tmp = NULL;
        img_tmp = IMG_Load(file_.c_str());
        if (!img_tmp && altFile_ != "")
        {
            //texture_ = IMG_LoadTexture(SDL::getRenderer(), altFile_.c_str());
	    img_tmp = IMG_Load(altFile_.c_str());
        }

        if (img_tmp != NULL)
        {

            /* Convert to RGB 32bit if necessary */
	    if(img_tmp->format->BitsPerPixel != 32){
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
	        //SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
	        /*SDL_QueryTexture(texture_, NULL, NULL, &width, &height);
		  baseViewInfo.ImageWidth = width * scaleX_;
		  baseViewInfo.ImageHeight = height * scaleY_;*/
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
    Component::draw();

    if(texture_)
    {
        SDL_Rect rect;

        rect.x = static_cast<int>(baseViewInfo.XRelativeToOrigin());
        rect.y = static_cast<int>(baseViewInfo.YRelativeToOrigin());
        rect.h = static_cast<int>(baseViewInfo.ScaledHeight());
        rect.w = static_cast<int>(baseViewInfo.ScaledWidth());

        SDL::renderCopy(texture_, baseViewInfo.Alpha, NULL, &rect, baseViewInfo);
    }
}
