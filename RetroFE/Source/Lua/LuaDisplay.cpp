#include "LuaDisplay.h"
#include "../SDL.h"

int LuaDisplay::getDimensions(lua_State *l)
{
    int h = SDL::getWindowHeight();
    int w = SDL::getWindowWidth();
    
    lua_pushnumber(l, w);
    lua_pushnumber(l, h);
    
    return 2;
}


int LuaDisplay::getWidth(lua_State *l)
{
    int w = SDL::getWindowWidth();
    
    lua_pushnumber(l, w);
    
    return 1;
}


int LuaDisplay::getHeight(lua_State *l)
{
    int h = SDL::getWindowHeight();
    
    lua_pushnumber(l, h);
    
    return 1;
}


int LuaDisplay::getCenter(lua_State *l)
{
    int h = SDL::getWindowHeight();
    int w = SDL::getWindowWidth();
    
    lua_pushnumber(l, w);
    lua_pushnumber(l, h);
    
    lua_pushnumber(l, w/2);
    lua_pushnumber(l, h/2);
    
    return 2;
}


int LuaDisplay::isFullscreen(lua_State *l)
{

    bool fullscreen = SDL::isFullscreen();
    
    lua_pushnumber(l, fullscreen);
    
    return 1;
}
