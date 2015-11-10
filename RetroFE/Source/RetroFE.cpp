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

#include "RetroFE.h"
#include "Utility/Log.h"
#include "Utility/Utils.h"
#include "SDL.h"
#include "Graphics/Component/Image.h"
#include "Graphics/Component/Component.h"
#include <vector>

#ifdef __linux
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>

#endif

#ifdef WIN32
#include <Windows.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_thread.h>
#endif


std::map<Component *, Component *> components;

static int lua_getCenter(lua_State *l)
{
    int x = SDL::getWindowWidth() / 2;
    int y = SDL::getWindowHeight() / 2;
    lua_pushnumber(l, x);
    lua_pushnumber(l, y);
    return 2;
}

static int lua_getDimensions(lua_State *l)
{
    int x = SDL::getWindowWidth();
    int y = SDL::getWindowHeight();
    lua_pushnumber(l, x);
    lua_pushnumber(l, y);
    return 2;
}

Image *i = NULL;
static int lua_imageCreate(lua_State *l)
{
    std::string filename = lua_tostring(l, 1);
    i = new Image(filename);
    i->Initialize();
    lua_pushinteger(l, (int)i);
    components[i] = i;
    return 1;
}

static int lua_imageDelete(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    if(components.find(i) != components.end()) {
        components.erase(i);
    }
    i->DeInitialize();
    delete i;
    return 0;
}

static int lua_imageSetSize(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    i->info.width = (int)lua_tointeger(l, 2);
    i->info.height = (int)lua_tointeger(l, 3);
    return 0;
}

static int lua_imageSetPosition(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    i->info.x = (int)lua_tointeger(l, 2);
    i->info.y = (int)lua_tointeger(l, 3);
    return 0;
}

static int lua_imageSetRotate(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    i->info.rotate = (float)lua_tonumber(l, 2);
    return 0;
}
static int lua_imageSetAlpha(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    i->info.alpha = (float)lua_tonumber(l, 2);
    return 0;
}

static int lua_imageAnimate(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    bool loop = (lua_toboolean(l, 2) != 0);

    i->animate(loop);
    return 0;
}

static int lua_imageAddAnimation(lua_State *l)
{
//    int argc = lua_gettop(l);
    Image *i = (Image *)lua_tointeger(l, 1);
    if(!i) return 0;

    float duration = (float)lua_tonumber(l, 2);

    if (!lua_istable(l, 3)) {
          return 0;
     }

    ComponentData newInfo;
    lua_pushstring (l, "x");
    lua_gettable(l, -2);
    newInfo.x = (int)luaL_optinteger(l, -1, i->info.x);
    lua_pop(l, 1);

    lua_pushstring (l, "y");
    lua_gettable(l, -2);
    newInfo.y = (int)luaL_optinteger(l, -1, i->info.y);
    lua_pop(l, 1);

    lua_pushstring (l, "alpha");
    lua_gettable(l, -2);
    newInfo.alpha = (float)luaL_optnumber(l, -1, i->info.alpha);
    lua_pop(l, 1);

    newInfo.duration = duration;
    i->addAnimation(newInfo);

    return 0;
}


static const luaL_Reg luaDisplayFuncs[] = {
  // Creation
  {"getCenter", lua_getCenter},
  {"getDimensions", lua_getDimensions},
  {NULL, NULL}
};

static const luaL_Reg luaImageFuncs[] = {
  // Creation
  {"create", lua_imageCreate},
  {"delete", lua_imageDelete},
  {"setSize", lua_imageSetSize},
  {"setRotate", lua_imageSetRotate},
  {"setPosition", lua_imageSetPosition},
  {"setAlpha", lua_imageSetAlpha},
  {"addAnimation", lua_imageAddAnimation},
  {"animate", lua_imageAnimate},
  {NULL, NULL}
};

void RetroFE::initializeLua()
{
    lua_.initialize();

    lua_newtable(lua_.state);
    luaL_setfuncs (lua_.state, luaDisplayFuncs, 0);
    lua_pushvalue(lua_.state,-1);        // pluck these lines out if they offend you
    lua_setglobal(lua_.state,"display"); // for they clobber the Holy _G

    lua_newtable(lua_.state);
    luaL_setfuncs (lua_.state, luaImageFuncs, 0);
    lua_pushvalue(lua_.state,-1);        // pluck these lines out if they offend you
    lua_setglobal(lua_.state,"image"); // for they clobber the Holy _G
}

void RetroFE::reloadLuaScripts()
{
    luaL_loadfile(lua_.state, "C:/Users/Don/Downloads/RetroFE-FTP/layouts/LUATest/Page.lua");
    lua_pcall(lua_.state, 0, LUA_MULTRET, 0);
}




RetroFE::RetroFE(Configuration &c)
: config_(c)
{
}

RetroFE::~RetroFE()
{
}



void RetroFE::run()
{
    if(!SDL::initialize(config_)) return;

    initializeLua();

    reloadLuaScripts();

    bool quit = false;
    double currentTime = 0;
    double lastTime = 0;
    double deltaTime = 0;
    while(!quit) {

        lastTime = currentTime;
        currentTime = static_cast<float>(SDL_GetTicks()) / 1000;

        if(lastTime == 0) 
        {
           lastTime = currentTime;
        }

        if (currentTime < lastTime)
        {
            currentTime = lastTime;
        }

        deltaTime = currentTime - lastTime;

        SDL_LockMutex(SDL::getMutex());
        SDL_SetRenderDrawColor(SDL::getRenderer(), 0x0, 0x0, 0x00, 0xFF);
        SDL_RenderClear(SDL::getRenderer());

        for(std::map<Component *, Component *>::iterator it = components.begin(); it != components.end(); it++)
        {
            it->second->update((float)deltaTime);
        }

        for(std::map<Component *, Component *>::iterator it = components.begin(); it != components.end(); it++)
        {
            it->second->draw();
        }

        SDL_RenderPresent(SDL::getRenderer());
        SDL_UnlockMutex(SDL::getMutex());
        
        double sleepTime = 1000.0/60.0 - deltaTime*1000;
        if(sleepTime > 0)
        {
            SDL_Delay(static_cast<unsigned int>(sleepTime));
        }
    }


}
