/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Font.h"
#include <string>
#include <map>

class FontCache
{
public:
    void Initialize();
    void DeInitialize();
    FontCache();
    bool LoadFont(std::string font, int fontSize, SDL_Color color);
    Font *GetFont(std::string font);

    virtual ~FontCache();
private:
    std::map<std::string, Font *> FontFaceMap;

};

