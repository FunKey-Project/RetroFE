#pragma once

#include "Lua.h"
#include "../Graphics/Component/ComponentFactory.h"

namespace LuaImage
{
    void initialize(ComponentFactory &f); 
    int create(lua_State *l);
    int destroy(lua_State *l);
    int loadFile(lua_State *l);
    int loadType(lua_State *l);
    int isLoaded(lua_State *l);
    int unload(lua_State *l);
    int getOriginalWidth(lua_State *l);
    int getOriginalHeight(lua_State *l);
    int getOriginalDimensions(lua_State *l);
    int getX(lua_State *l);
    int getY(lua_State *l);
    int getPosition(lua_State *l);
    int getWidth(lua_State *l);
    int getHeight(lua_State *l);
    int getDimensions(lua_State *l);
    int getRotate(lua_State *l);
    int getAlpha(lua_State *l);
    int getLayer(lua_State *l);
    int getHeight(lua_State *l);
    int setX(lua_State *l);
    int setY(lua_State *l);
    int setPosition(lua_State *l);
    int setWidth(lua_State *l);
    int setHeight(lua_State *l);
    int setDimensions(lua_State *l);
    int setRotate(lua_State *l);
    int setAlpha(lua_State *l);
    int setLayer(lua_State *l);
    int setHeight(lua_State *l);    
};
