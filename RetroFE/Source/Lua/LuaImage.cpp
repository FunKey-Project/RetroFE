#include "LuaImage.h"
#include "../Utility/Log.h"
#include "../Graphics/Component/Image.h"


int LuaImage::create(lua_State *l)
{
    Image *i = new Image();
    
    lua_pushinteger(l, (int)i);
    
    return 1; 
}

int LuaImage::destroy(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    delete i;
    
    return 0; 
}

int LuaImage::loadFile(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    std::string file = luaL_checkstring(l, 2);
    
    bool result = i->load(file);
    lua_pushboolean(l, result);

    return 1; 
}

#if 0
int LuaImage::loadType(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    stdL::string type = luaL_checkstring(l, 2);
    stdL::string item = luaL_checkstring(l, 3);
    
    bool result = i->loadType(type, item);
    lua_pushboolean(l, result);

    return 2; 
}


int LuaImage::isLoaded(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushboolean(l, i->isLoaded());

    return 1; 
}


int LuaImage::unload(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    i->unload();
    
    return 0; 
}


int LuaImage::getOriginalWidth(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->getOriginalWidth());
    
    return 1; 
}


int LuaImage::getOriginalHeight(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->getOriginalHeight());
    
    return 1;  
}


int LuaImage::getOriginalDimensions(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->getOriginalWidth());
    lua_pushnumber(l, i->getOriginalHeight());
    
    return 2; 
}


int LuaImage::getX(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.x);
    
    return 1; 
}


int LuaImage::getY(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.y);
    
    return 1; 
}


int LuaImage::getPosition(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.x);
    lua_pushnumber(l, i->info.y);
    
    return 2; 
}


int LuaImage::getWidth(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.width);
    
    return 1; 
}


int LuaImage::getHeight(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.height);
    
    return 1; 
}


int LuaImage::getDimensions(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.width);
    lua_pushnumber(l, i->info.height);
    
    return 2; 
}


int LuaImage::getRotate(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.rotate);
    
    return 1; 
}


int LuaImage::getAlpha(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, i->info.alpha);
    
    return 1; 
}


int LuaImage::getVisible(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushboolean(l, i->info.visible);
    
    return 1; 
}


int LuaImage::getHidden(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushboolean(l, !i->info.visible);
    
    return 1; 
}


int LuaImage::getLayer(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumberl, !i->info.layer);
    
    return 1; 
}

int LuaImage::setX(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = luaL_checknumber(l, 2);
    
    i->info.x = val;
    
    return 0; 
}


int LuaImage::setY(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = luaL_checknumber(l, 2);
    
    i->info.y = val;
    
    return 0; 
}


int LuaImage::setPosition(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int x = luaL_checknumber(l, 2);
    int y = luaL_checknumber(l, 3);
    
    i->info.x = x;
    i->info.y = y;
    
    return 0; 
}


int LuaImage::setWidth(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = luaL_checknumber(l, 2);
    
    i->info.width = val;
    
    return 0; 
}


int LuaImage::setHeight(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = luaL_checknumber(l, 2);
    
    i->info.height = val;
    
    return 0; 
}


int LuaImage::setDimensions(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int w = luaL_checknumber(l, 2);
    int h = luaL_checknumber(l, 3);
    
    i->info.width = w;
    i->info.height = h;
    
    return 0; 
}


int LuaImage::setRotate(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    float val = luaL_checknumber(l, 2);
    
    i->info.rotate = val;
    
    return 0; 
}


int LuaImage::setAlpha(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    float val = luaL_checknumber(l, 2);
    
    i->info.alpha = val;
    
    return 0; 
}


int LuaImage::setVisible(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    bool val = luaL_checkboolean(l, 2);
    
    i->info.visible = val;
    
    return 0; 
}


int LuaImage::setHidden(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    bool val = luaL_checkboolean(l, 2);
    
    i->info.visible = !val;
    
    return 0; 
}


int LuaImage::setLayer(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = luaL_checknumber(l, 2);
    
    i->info.layer = val;
    
    return 0; 
}

#endif