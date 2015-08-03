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
        int minX;
        int maxX;
        int minY;
        int maxY;
        int advance;
        SDL_Rect rect;
    };

    Font();
    virtual ~Font();
    bool initialize(std::string fontPath, int fontSize, SDL_Color color);
    void deInitialize();
    SDL_Texture *getTexture();
    bool getRect(unsigned int charCode, GlyphInfo &glyph);
    int getHeight();
    int getAscent();

private:
    struct GlyphInfoBuild
    {
        Font::GlyphInfo glyph;
        SDL_Surface *surface;
    };

    SDL_Texture *texture;
    int height;
    int ascent;
    std::map<unsigned int, GlyphInfoBuild *> atlas;
};
