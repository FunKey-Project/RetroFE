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
#include <map>
#include <string>

class Font
{
public:
    struct GlyphInfo
    {
        int MinX;
        int MaxX;
        int MinY;
        int MaxY;
        int Advance;
        SDL_Rect Rect;
    };

    Font();
    virtual ~Font();
    bool Initialize(std::string fontPath, int fontSize, SDL_Color color);
    void DeInitialize();
    SDL_Texture *GetTexture();
    bool GetRect(unsigned int charCode, GlyphInfo &glyph);


private:
    struct GlyphInfoBuild
    {
        Font::GlyphInfo Glyph;
        SDL_Surface *Surface;
    };

    std::map<unsigned int, GlyphInfoBuild> Atlas;
    SDL_Texture *Texture;
};
