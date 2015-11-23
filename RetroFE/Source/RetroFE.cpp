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
#include "StateMachine.h"
#include "Utility/Log.h"
#include "Utility/Utils.h"
#include "SDL.h"
#include "Graphics/Component/Image.h"
#include "Graphics/Component/Component.h"
#include "Lua/LuaCollection.h"
#include "Lua/LuaDisplay.h"
#include "Lua/LuaImage.h"
#include "Lua/LuaLog.h"
#include "Lua/LuaEvent.h"
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


static int lua_registerOnInit(lua_State *l)
{
//    std::string function = lua_tostring(l, 1);
//    events.registerOnInit(function);
    return 0;
}

const luaL_Reg RetroFE::luaImageFuncs[] = {
  // Creation
  {"create", LuaImage::create},
  {"loadFile", LuaImage::loadFile},
  {"loadType", LuaImage::loadType},
  {"getOriginalWidth", LuaImage::getOriginalWidth},
  {"getOriginalHeight", LuaImage::getOriginalHeight},
  {"getOriginalDimensions", LuaImage::getOriginalDimensions},
  {"getX", LuaImage::getX},
  {"getY", LuaImage::getY},
  {"getPosition", LuaImage::getPosition},
  {"getWidth", LuaImage::getWidth},
  {"getHeight", LuaImage::getHeight},
  {"getDimensions", LuaImage::getDimensions},
  {"getRotate", LuaImage::getRotate},
  {"getAlpha", LuaImage::getAlpha},
  {"setX", LuaImage::setX},
  {"setY", LuaImage::setY},
  {"setPosition", LuaImage::setPosition},
  {"setWidth", LuaImage::setWidth},
  {"setHeight", LuaImage::setHeight},
  {"setDimensions", LuaImage::setDimensions},
  {"setRotate", LuaImage::setRotate},
  {"setAlpha", LuaImage::setAlpha},
  {"addAnimation", LuaImage::addAnimation},
  {"animate", LuaImage::animate},
  {"destroy", LuaImage::destroy},
  {NULL, NULL}
};

const luaL_Reg RetroFE::luaCollectionFuncs[] = {
    {"load", LuaCollection::load},
    {"destroy", LuaCollection::destroy},
    {"getSize", LuaCollection::getSize},
    {"getName", LuaCollection::getName},
    {"getItemAt", LuaCollection::getItemAt},
    {NULL, NULL}
};

const luaL_Reg RetroFE::luaDisplayFuncs[] = {
    {"getCenter", LuaDisplay::getCenter},
    {"getDimensions", LuaDisplay::getDimensions},
    {NULL, NULL}
};

const luaL_Reg RetroFE::luaLogFuncs[] = {
    {"debug", LuaLog::debug},
    {"info", LuaLog::info},
    {"notice", LuaLog::notice},
    {"warning", LuaLog::warning},
    {"error", LuaLog::error},
    {NULL, NULL}
};

void RetroFE::initializeLua()
{
    lua_.initialize();
    LuaImage::initialize(&config_, factory_);
    LuaCollection::initialize(&config_, cib_, &luaEvent_);

    lua_newtable(lua_.state);
    luaL_setfuncs (lua_.state, luaCollectionFuncs, 0);
    lua_pushvalue(lua_.state, -1);
    lua_setglobal(lua_.state, "collection");

    lua_newtable(lua_.state);
    luaL_setfuncs (lua_.state, luaDisplayFuncs, 0);
    lua_pushvalue(lua_.state, -1);
    lua_setglobal(lua_.state, "display");

    lua_newtable(lua_.state);
    luaL_setfuncs (lua_.state, luaImageFuncs, 0);
    lua_pushvalue(lua_.state, -1);
    lua_setglobal(lua_.state, "image");
    
    lua_newtable(lua_.state);
    luaL_setfuncs (lua_.state, luaLogFuncs, 0);
    lua_pushvalue(lua_.state, -1);
    lua_setglobal(lua_.state, "log");
    
}

void RetroFE::reloadLuaScripts()
{
    std::string path = config_.absolutePath + "/layouts/LUATest/Page.lua";
    luaL_loadfile(lua_.state, path.c_str());
    lua_pcall(lua_.state, 0, LUA_MULTRET, 0);

}

RetroFE::RetroFE(Configuration &c)
: config_(c)
, db_(c.absolutePath + "/meta.db")
, mdb_(NULL)
, cib_(NULL)
{
}

RetroFE::~RetroFE()
{
    if(cib_) delete cib_;
    if(mdb_) delete mdb_;
}



void RetroFE::run()
{
    if(!SDL::initialize(config_)) return;
    
    db_.initialize();
    mdb_ = new MetadataDatabase(db_, config_);
    cib_ = new CollectionInfoBuilder(config_, *mdb_);
    mdb_->initialize();
    initializeLua();
    StateMachine state(lua_.state, &luaEvent_);

    reloadLuaScripts();
//    events.triggerOnInit(lua_.state);
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

        LuaCollection::update(lua_.state);

        state.update((float)deltaTime);

        SDL_LockMutex(SDL::getMutex());
        SDL_SetRenderDrawColor(SDL::getRenderer(), 0x0, 0x0, 0x00, 0xFF);
        SDL_RenderClear(SDL::getRenderer());

        for(std::map<Component *, Component *>::iterator it = factory_.components.begin(); it != factory_.components.end(); it++)
        {
            it->second->update((float)deltaTime);
        }

        for(std::map<Component *, Component *>::iterator it = factory_.components.begin(); it != factory_.components.end(); it++)
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