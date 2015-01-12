/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
    bool Initialize(std::string fontPath);
    void DeInitialize();
    SDL_Texture *GetTexture();
    bool GetRect(unsigned int charCode, GlyphInfo &glyph);


private:
    struct GlyphInfoBuild
    {
        Font::GlyphInfo Glyph;
        SDL_Surface *Surface;
    };

    std::map<unsigned int, GlyphInfoBuild *> Atlas;
    SDL_Texture *Texture;
};


