/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
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

bool FontCache::LoadFont(std::string fontPath, SDL_Color color)
{
    std::map<std::string, Font *>::iterator it = FontFaceMap.find(fontPath);

    if(it == FontFaceMap.end())
    {
        Font *f = new Font();
        f->Initialize(fontPath, color);
        FontFaceMap[fontPath] = f;
    }

    return true;
}

