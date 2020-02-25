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

#include "Component.h"
#include <SDL/SDL.h>
#include <string>

class Image : public Component
{
public:
    Image(std::string file, std::string altFile, Page &p, float scaleX, float scaleY, bool dithering);
    virtual ~Image();
    void freeGraphicsMemory();
    void allocateGraphicsMemory();
    void draw();

protected:
    SDL_Surface *texture_;
    SDL_Surface *texture_prescaled_;
    std::string file_;
    std::string altFile_;
    float scaleX_;
    float scaleY_;
    bool ditheringAuthorized_;
    bool needDithering_;
    int imgBitsPerPx_;
};
