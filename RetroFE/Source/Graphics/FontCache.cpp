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

#include "FontCache.h"
#include "Font.h"
#include "../Utility/Log.h"
#include "../SDL.h"
#include <SDL2/SDL_ttf.h>
#include <sstream>

//todo: memory leak when launching games
FontCache::FontCache()
{
}

FontCache::~FontCache()
{
    DeInitialize();
}

void FontCache::DeInitialize()
{
    std::map<std::string, Font *>::iterator it = FontFaceMap.begin();
    while(it != FontFaceMap.end())
    {
        delete it->second;
        FontFaceMap.erase(it);
        it = FontFaceMap.begin();
    }

    SDL_LockMutex(SDL::GetMutex());
    TTF_Quit();
    SDL_UnlockMutex(SDL::GetMutex());
}


void FontCache::Initialize()
{
    //todo: make bool
    TTF_Init();
}
Font *FontCache::GetFont(std::string fontPath)
{
    Font *t = NULL;

    std::map<std::string, Font *>::iterator it = FontFaceMap.find(fontPath);

    if(it != FontFaceMap.end())
    {
        t = it->second;
    }

    return t;
}

bool FontCache::LoadFont(std::string fontPath, int fontSize, SDL_Color color)
{
    std::map<std::string, Font *>::iterator it = FontFaceMap.find(fontPath);

    if(it == FontFaceMap.end())
    {
        Font *f = new Font();
        f->Initialize(fontPath, fontSize, color);
        FontFaceMap[fontPath] = f;
    }

    return true;
}

