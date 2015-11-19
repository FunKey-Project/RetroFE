#include "LuaEvent.h"

void LuaEvent::registerOnInit(std::string functionName)
{
  onInit_.push_back(functionName);
}

void LuaEvent::triggerOnInit(lua_State *l)
{
  for(unsigned int i = 0;  i < onInit_.size(); ++i) {
    lua_getglobal(l, onInit_[i].c_str());
    lua_call(l, 0, 0);
  }

  // arguments
	//lua_pushnumber(l, x);
	//lua_pushnumber(l, y);
	// call the function with 2 arguments, return 1 result
	//lua_call(L, 2, 1);
	//sum = (int)lua_tointeger(L, -1);
	//lua_pop(L, 1);
}
