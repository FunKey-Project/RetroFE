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
#include "Font.h"
#include "../SDL.h"
#include "../Utility/Log.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

Font::Font()
    : Texture(NULL)
{
}

Font::~Font()
{
    DeInitialize();
}

SDL_Texture *Font::GetTexture()
{
    return Texture;
}

bool Font::GetRect(unsigned int charCode, GlyphInfo &glyph)
{
    std::map<unsigned int, GlyphInfoBuild *>::iterator it = Atlas.find(charCode);

    if(it != Atlas.end())
    {
        GlyphInfoBuild *info = it->second;

        glyph = info->Glyph;

        return true;
    }

    return false;
}

void SetSurfaceAlpha (SDL_Surface *surface, Uint8 alpha)
{
    SDL_PixelFormat* fmt = surface->format;

    // If surface has no alpha channel, just set the surface alpha.
    if( fmt->Amask == 0 ) {
        SDL_SetSurfaceAlphaMod( surface, alpha );
    }
    // Else change the alpha of each pixel.
    else {
        unsigned bpp = fmt->BytesPerPixel;
        // Scaling factor to clamp alpha to [0, alpha].
        float scale = alpha / 255.0f;

        SDL_LockSurface(surface);

        for (int y = 0; y < surface->h; ++y) 
        for (int x = 0; x < surface->w; ++x) {
            // Get a pointer to the current pixel.
            Uint32* pixel_ptr = (Uint32 *)( 
                    (Uint8 *)surface->pixels
                    + y * surface->pitch
                    + x * bpp
                    );

            // Get the old pixel components.
            Uint8 r, g, b, a;
            SDL_GetRGBA( *pixel_ptr, fmt, &r, &g, &b, &a );

            // Set the pixel with the new alpha.
            *pixel_ptr = SDL_MapRGBA( fmt, r, g, b, (Uint8)(scale * a) );
        }   

        SDL_UnlockSurface(surface);
    }       
}       

bool Font::Initialize(std::string fontPath, SDL_Color color)
{
    TTF_Font *font = TTF_OpenFont(fontPath.c_str(), 128);

    if (!font)
    {
        Logger::Write(Logger::ZONE_ERROR, "FontCache", "TTF_OpenFont failed");
        return false;
    }

    int x = 0;
    int y = 0;
    int atlasHeight = 0;
    int atlasWidth = 0;

    for(unsigned short int i = 32; i < 128; ++i)
    {
        GlyphInfoBuild *info = new GlyphInfoBuild;
        memset(info, sizeof(GlyphInfoBuild), 0);

        color.a = 255;
        info->Surface = TTF_RenderGlyph_Blended(font, i, color);
        TTF_GlyphMetrics(font, i, &info->Glyph.MinX, &info->Glyph.MaxX, &info->Glyph.MinY, &info->Glyph.MaxY, &info->Glyph.Advance);

        if(x + info->Surface->w >= 1024)
        {
            atlasHeight += y;
            atlasWidth = (atlasWidth >= x) ? atlasWidth : x;
            x = 0;
            y = 0;
        }

        info->Glyph.Rect.w = info->Surface->w;
        info->Glyph.Rect.h = info->Surface->h;
        info->Glyph.Rect.x = x;
        info->Glyph.Rect.y = atlasHeight;
        Atlas[i] = info;

        x += info->Glyph.Rect.w;
        y = (y > info->Glyph.Rect.h) ? y : info->Glyph.Rect.h;
        /*
        std::stringstream ss;
        ss << " tw:" << atlasWidth << " th:" << atlasHeight << " x:" << x << " y:" << y << " w:" << info->Glyph.Rect.w << " h:" << info->Glyph.Rect.h;
        Logger::Write(Logger::ZONE_ERROR, "FontCache", ss.str());
                */
    }

    atlasWidth = (atlasWidth >= x) ? atlasWidth : x;
    atlasHeight += y;

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

    SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0, atlasWidth, atlasHeight, 32, rmask, gmask, bmask, amask);
    std::map<unsigned int, GlyphInfoBuild *>::iterator it;
    for(it = Atlas.begin(); it != Atlas.end(); it++)
    {
        GlyphInfoBuild *info = it->second;
        SDL_BlitSurface(info->Surface, NULL, atlasSurface, &info->Glyph.Rect);
        SDL_FreeSurface(info->Surface);
        info->Surface = NULL;
    }
    SDL_LockMutex(SDL::GetMutex());

    Texture = SDL_CreateTextureFromSurface(SDL::GetRenderer(), atlasSurface);
    SDL_FreeSurface(atlasSurface);
    SDL_UnlockMutex(SDL::GetMutex());

    TTF_CloseFont(font);

    return true;
}



void Font::DeInitialize()
{
    if(Texture)
    {
        SDL_LockMutex(SDL::GetMutex());
        SDL_DestroyTexture(Texture);
        Texture = NULL;
        SDL_UnlockMutex(SDL::GetMutex());
    }

    std::map<unsigned int, GlyphInfoBuild *>::iterator atlasIt = Atlas.begin();
    while(atlasIt != Atlas.end())
    {
        delete atlasIt->second;
        Atlas.erase(atlasIt);
        atlasIt = Atlas.begin();
    }

}
