#include "LuaImage.h"
#include "../Utility/Log.h"

ComponentFactory *factory;

void LuaImage::initialize(ComponentFactory &f) 
{
    factory = &f;
}

int LuaImage::create(lua_State *l)
{
    Image *i = factory->createImage();
    
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
#endif

int LuaImage::getOriginalWidth(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int w = 0;
    int h = 0;

    i->getOriginalDimensions(w, h);
    lua_pushnumber(l, w);
    
    return 1; 
}


int LuaImage::getOriginalHeight(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int w = 0;
    int h = 0;

    i->getOriginalDimensions(w, h);
    lua_pushnumber(l, h);
    
    return 1; 
}


int LuaImage::getOriginalDimensions(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int w = 0;
    int h = 0;

    i->getOriginalDimensions(w, h);
    lua_pushnumber(l, w);
    lua_pushnumber(l, h);
    
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

int LuaImage::getLayer(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    
    lua_pushnumber(l, !i->info.layer);
    
    return 1; 
}

int LuaImage::setX(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = (int)luaL_checknumber(l, 2);
    
    i->info.x = val;
    
    return 0; 
}


int LuaImage::setY(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = (int)luaL_checknumber(l, 2);
    
    i->info.y = val;
    
    return 0; 
}


int LuaImage::setPosition(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int x = (int)luaL_checknumber(l, 2);
    int y = (int)luaL_checknumber(l, 3);
    
    i->info.x = x;
    i->info.y = y;
    
    return 0; 
}


int LuaImage::setWidth(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = (int)luaL_checknumber(l, 2);
    
    i->info.width = val;
    
    return 0; 
}


int LuaImage::setHeight(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = (int)luaL_checknumber(l, 2);
    
    i->info.height = val;
    
    return 0; 
}


int LuaImage::setDimensions(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int w = (int)luaL_checknumber(l, 2);
    int h = (int)luaL_checknumber(l, 3);
    
    i->info.width = w;
    i->info.height = h;
    
    return 0; 
}


int LuaImage::setRotate(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    float val = (float)luaL_checknumber(l, 2);
    
    i->info.rotate = val;
    
    return 0; 
}


int LuaImage::setAlpha(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    float val = (float)luaL_checknumber(l, 2);
    
    i->info.alpha = val;
    
    return 0; 
}

int LuaImage::setLayer(lua_State *l)
{
    Image *i = (Image *)luaL_checkinteger(l, 1);
    int val = (int)luaL_checknumber(l, 2);
    
    i->info.layer = val;
    
    return 0; 
}

int LuaImage::animate(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    bool loop = (lua_toboolean(l, 2) != 0);

    i->animate(loop);
    return 0;
}

template <class T>
static void BuildTableComponentInfo(lua_State *l, std::string name, unsigned char mask, ComponentData &newInfo, T &newParam) {
    lua_pushstring (l, name.c_str());
    lua_gettable(l, -2);
    if(!lua_isnil(l, -1)) 
    {
        newInfo.setMask(mask);
        newParam = (T)luaL_checknumber(l, -1);
    }
    lua_pop(l, 1);
}

int LuaImage::addAnimation(lua_State *l)
{
    Image *i = (Image *)lua_tointeger(l, 1);
    if(!i) return 0;

    float duration = (float)lua_tonumber(l, 2);

    if (!lua_istable(l, 3)) {
          return 0;
     }

    ComponentData newInfo;
    newInfo.clearMask(COMPONENT_DATA_ALL_MASK);

    BuildTableComponentInfo<int>(l, "x", COMPONENT_DATA_X_MASK, newInfo, newInfo.x);
    BuildTableComponentInfo<int>(l, "y", COMPONENT_DATA_Y_MASK, newInfo, newInfo.y);
    BuildTableComponentInfo<int>(l, "height", COMPONENT_DATA_Y_MASK, newInfo, newInfo.height);
    BuildTableComponentInfo<int>(l, "width", COMPONENT_DATA_Y_MASK, newInfo, newInfo.width);
    BuildTableComponentInfo<float>(l, "rotate", COMPONENT_DATA_ROTATE_MASK, newInfo, newInfo.rotate);
    BuildTableComponentInfo<float>(l, "alpha", COMPONENT_DATA_ROTATE_MASK, newInfo, newInfo.alpha);

    newInfo.duration = duration;
    i->addAnimation(newInfo);

    return 0;
}

