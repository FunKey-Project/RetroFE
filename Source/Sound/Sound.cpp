/* This file is subject to the terms and conditions defined in
 * file 'LICENSE.txt', which is part of this source code package.
 */
#include "Sound.h"

#include "../Utility/Log.h"

Sound::Sound(std::string file)
: File(file)
, Chunk(NULL)
{
   if(!Allocate())
   {
      Logger::Write(Logger::ZONE_ERROR, "Sound", "Cannot load " + File);
   }
}

Sound::~Sound()
{
   if(Chunk)
   {
      Mix_FreeChunk(Chunk);
      Chunk = NULL;
   }
}

void Sound::Play()
{
   if(Chunk)
   {
      (void)Mix_PlayChannel(-1, Chunk, 0);
   }
}

bool Sound::Free()
{
   if(Chunk)
   {
      Mix_FreeChunk(Chunk);
      Chunk = NULL;
   }

   return true;
}

bool Sound::Allocate()
{
   if(!Chunk)
   {
      Chunk = Mix_LoadWAV(File.c_str());
   }

   return (Chunk != NULL);
}
