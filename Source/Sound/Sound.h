/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#pragma once

#include <string>
#include <SDL2/SDL_mixer.h>
class Sound
{
public:
   Sound(std::string file);
   virtual ~Sound();
   void Play();
   bool Allocate();
   bool Free();
private:
   std::string File;
   Mix_Chunk *Chunk;
};
