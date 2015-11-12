#include "Lua.h"
#include <string>

Lua::Lua()
    : state(NULL)
{
}

void Lua::initialize()
{
	/* initialize Lua */
    state = luaL_newstate();

	/* load Lua base libraries */
	luaL_openlibs(state);

    lua_getglobal( state, "package" );
    lua_getfield( state, -1, "path" ); // get field "path" from table at top of stack (-1)
    std::string cur_path = lua_tostring( state, -1 ); // grab path string from top of stack
    cur_path.append( ";" ); // do your path magic here
    cur_path.append( "../assets/?.lua;" );
    lua_pop( state, 1 ); // get rid of the string on the stack we just pushed on line 5
    lua_pushstring( state, cur_path.c_str() ); // push the new one
    lua_setfield( state, -2, "path" ); // set the field "path" in table at -2 with value at top of stack
    lua_pop( state, 1 ); // get rid of package table from top of stack
}

void Lua::deInitialize()
{
    if(state)
    {
    	lua_close(state);
        state = NULL;
    }
}
