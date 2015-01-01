/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <SDL2/SDL.h>
#include <string>

class IVideo
{
public:
   virtual ~IVideo() {}
   virtual bool Initialize() = 0;
   virtual bool Play(std::string file) = 0;
   virtual bool Stop() = 0;
   virtual bool DeInitialize() = 0;
   virtual SDL_Texture *GetTexture() const = 0;
   virtual void Update(float dt) = 0;
   virtual void Draw() = 0;
};
