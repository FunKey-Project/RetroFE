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
#include "../Utility/Utils.h"

SDL_TimerID Sound::idTimer = 0;
int Sound::ampliStarted = 0;

Sound::Sound(std::string file, std::string altfile)
    : file_(file)
    , chunk_(NULL)
    , channel_(-1)
{
    if(!allocate())
    {
        file_ = altfile;
        if (!allocate())
        {
            Logger::write(Logger::ZONE_ERROR, "Sound", "Cannot load " + file_);
        }
    }
}

Sound::~Sound()
{
    if(chunk_)
    {
        Mix_FreeChunk(chunk_);
        chunk_ = NULL;
    }
}

void Sound::play()
{
    FILE *fp;

    //printf("%s\n", __func__);
    SDL_RemoveTimer(idTimer);
    if(!ampliStarted){
        fp = popen(SHELL_CMD_TURN_AMPLI_ON, "r");
	if (fp != NULL) {
	    ampliStarted = 1;
	    pclose(fp);
	}
    }
    
    if(chunk_)
    {
        channel_ = Mix_PlayChannel(-1, chunk_, 0);
        Mix_ChannelFinished(finished);
    }
}

uint32_t Sound::turnOffAmpli(uint32_t interval, void *param)
{
    FILE *fp;

    //printf("%s\n", __func__);
    fp = popen(SHELL_CMD_TURN_AMPLI_OFF, "r");
    if (fp != NULL) {
        ampliStarted = 0;
	pclose(fp);
    }
    return 0;
}

void Sound::finished(int channel)
{
    //printf("%s\n", __func__);
    if((channel == -1) || !Mix_Playing(channel)){
        SDL_RemoveTimer(idTimer);
        idTimer = SDL_AddTimer(500, turnOffAmpli, NULL);
    }
}

bool Sound::free()
{
    //printf("%s\n", __func__);
    if(chunk_)
    {
        Mix_FreeChunk(chunk_);
        chunk_   = NULL;
        channel_ = -1;
    }

    return true;
}

bool Sound::allocate()
{
    //printf("%s\n", __func__);
    if(!chunk_)
    {
        chunk_ = Mix_LoadWAV(file_.c_str());
    }

    return (chunk_ != NULL);
}


bool Sound::isPlaying()
{
    //printf("%s\n", __func__);
    return (channel_ != -1) && Mix_Playing(channel_);
}
