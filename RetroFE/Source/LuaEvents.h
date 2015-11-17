#pragma once
#include "Lua.h"
#include <string>
#include <vector>

class LuaEvents {
public:
  void registerOnInit(std::string functionName);
  void triggerOnInit(lua_State *l);
private:  
  std::vector<std::string> onInit_;
};