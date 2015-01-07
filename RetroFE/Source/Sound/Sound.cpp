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
