/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Component.h"

#include <SDL2/SDL.h>
#include <vector>

class Font;

class Text : public Component
{
public:
    //todo: should have a Font flass that references fontcache, pass that in as an argument
    Text(std::string text, Font *font, SDL_Color fontColor, float scaleX, float scaleY);
    virtual ~Text();
    void AllocateGraphicsMemory();
    void FreeGraphicsMemory();
    void Draw();

private:
    std::string TextData;
    Font *FontInst;
    SDL_Color FontColor;
    float ScaleX;
    float ScaleY;
};
