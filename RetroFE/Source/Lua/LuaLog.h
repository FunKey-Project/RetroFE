#pragma once

#include "Lua.h"

namespace LuaLog
{
    int debug(lua_State *l);
    int info(lua_State *l);
    int notice(lua_State *l);
    int warning(lua_State *l);
    int error(lua_State *l);
};