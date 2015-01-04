/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include "Component.h"
#include <SDL2/SDL.h>
#include <string>

class Image : public Component
{
public:
    Image(std::string file, float scaleX, float scaleY);
    virtual ~Image();
    void FreeGraphicsMemory();
    void AllocateGraphicsMemory();
    void Draw();

protected:
    SDL_Texture *Texture;
    std::string File;
    float ScaleX;
    float ScaleY;
};
