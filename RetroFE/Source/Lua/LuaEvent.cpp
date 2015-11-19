#include "LuaEvent.h"

void LuaEvent::registerCallback(std::string type, std::string functionName, std::string complete)
{
    callbacks_[type] = functionName;
    completeCallbacks_[type] = complete;
}


void LuaEvent::trigger(lua_State *l, std::string type)
{
    lua_getglobal(l, callbacks_[type].c_str());
    lua_call(l, 0, 0);
}

bool LuaEvent::isComplete(lua_State *l, std::string type)
{
    bool retval = true;
    lua_getglobal(l, callbacks_[type].c_str());
    if(lua_pcall(l, 0, 1, 0) == 0) 
    {
        if(!lua_isnil(l, 1)) 
        {
            if(!lua_toboolean(l, 1)) 
            {
                retval = false;
            }
        }
        lua_pop(l, 1);
    }

    return retval;
  // arguments
	//lua_pushnumber(l, x);
	//lua_pushnumber(l, y);
	// call the function with 2 arguments, return 1 result
	//lua_call(L, 2, 1);
	//sum = (int)lua_tointeger(L, -1);
	//lua_pop(L, 1);
}

