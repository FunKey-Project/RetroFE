#pragma once
#include "Lua.h"
#include <string>
#include <map>

class LuaEvent {
public:
  void trigger(lua_State *l, std::string type);
  void trigger(lua_State *l, std::string type, void *value);
  bool isBusy(lua_State *l, std::string type);
};