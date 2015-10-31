#pragma once

#include <lua.hpp>
extern "C" {
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}


class Lua
{
public:
    Lua();
    void initialize();
    void deInitialize();
    lua_State* state;

};
