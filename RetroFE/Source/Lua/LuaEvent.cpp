#include "LuaEvent.h"

void LuaEvent::trigger(lua_State *l, std::string type)
{
    lua_getglobal(l, type.c_str());

    lua_pcall(l, 0, 0, 0);
}

void LuaEvent::trigger(lua_State *l, std::string type, void *value)
{
    lua_getglobal(l, type.c_str());
    lua_pushlightuserdata(l, value);

    lua_pcall(l, 1, 0, 0);
}



bool LuaEvent::isBusy(lua_State *l, std::string type)
{
    bool retval = false;
    lua_getglobal(l, type.c_str());
    if(lua_pcall(l, 0, 1, 0) == 0) 
    {
        if(lua_isboolean(l, -1)) 
        {
            if(lua_toboolean(l, -1)) 
            {
                retval = true;
            }
        }
        lua_pop(l, 1);
    }

    return retval;
}

