#pragma once
#include "Lua.h"
#include <string>
#include <map>

class LuaEvent {
public:
  void registerCallback(std::string type, std::string functionName, std::string complete);
  void trigger(lua_State *l, std::string type);
  bool isComplete(lua_State *l, std::string type);
private:  
  std::map<std::string, std::string> callbacks_;
  std::map<std::string, std::string> completeCallbacks_;
};