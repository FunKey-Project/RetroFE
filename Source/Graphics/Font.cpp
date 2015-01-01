/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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
   SDL_Surface *atlasSurface = SDL_CreateRGBSurface(0, atlasWidth, atlasHeight, 24, rmask, gmask, bmask, amask);

   std::map<unsigned int, GlyphInfoBuild *>::iterator it;
   for(it = Atlas.begin(); it != Atlas.end(); it++)
   {
      GlyphInfoBuild *info = it->second;
      SDL_BlitSurface(info->Surface, NULL, atlasSurface, &info->Glyph.Rect);
      SDL_FreeSurface(info->Surface);
      info->Surface = NULL;
   }

   SDL_LockMutex(SDL::GetMutex());
   SDL_SetColorKey(atlasSurface, SDL_TRUE, SDL_MapRGB(atlasSurface->format, 0, 0, 0));
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
