#include "LuaLog.h"
#include "../Utility/Log.h"
#include <string>

int LuaLog::debug(lua_State *l)
{
    std::string message = luaL_checkstring(l, 1);

    Logger::write(Logger::ZONE_DEBUG, "Script", message);
    
    return 0; 
}

int LuaLog::info(lua_State *l)
{
    std::string message = luaL_checkstring(l, 1);

    Logger::write(Logger::ZONE_INFO, "Script", message);
    
    return 0; 
}

int LuaLog::warning(lua_State *l)
{
    std::string message = luaL_checkstring(l, 1);

    Logger::write(Logger::ZONE_WARNING, "Script", message);
    
    return 0; 
}

int LuaLog::notice(lua_State *l)
{
    std::string message = luaL_checkstring(l, 1);

    Logger::write(Logger::ZONE_NOTICE, "Script", message);
    
    return 0; 
}

int LuaLog::error(lua_State *l)
{
    std::string message = luaL_checkstring(l, 1);

    Logger::write(Logger::ZONE_ERROR, "Script", message);
    
    return 0; 
}