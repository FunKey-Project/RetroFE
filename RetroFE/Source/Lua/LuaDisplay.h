#pragma once

#include "Lua.h"

namespace LuaDisplay
{
    int getDimensions(lua_State *l);
    int getWidth(lua_State *l);
    int getHeight(lua_State *l);
    int getCenter(lua_State *l);
    int isFullscreen(lua_State *l);
    
};
