#include "LuaAnimate.h"
#include "../Utility/Log.h"
#include "../Utility/Utils.h"
#include "../Graphics/Component/Component.h"
#include "../Graphics/Animate/Animation.h"
#include "../Graphics/Animate/TweenTypes.h"
#include "../Graphics/Animate/Tween.h"


AnimationManager *manager;

void LuaAnimate::initialize(AnimationManager *m)
{
    manager = m;
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


Animation *LuaAnimate::create(lua_State *l)
{
    if (!lua_istable(l, 3)) { return NULL; }

    //todo: make sure it is a component
    Animation *a = new Animation();

    a->end.clearMask(COMPONENT_DATA_ALL_MASK);
    BuildTableComponentInfo<int>(l, "x", COMPONENT_DATA_X_MASK, a->end, a->end.x);
    BuildTableComponentInfo<int>(l, "y", COMPONENT_DATA_Y_MASK, a->end, a->end.y);
    BuildTableComponentInfo<int>(l, "height", COMPONENT_DATA_Y_MASK, a->end, a->end.height);
    BuildTableComponentInfo<int>(l, "width", COMPONENT_DATA_Y_MASK, a->end, a->end.width);
    BuildTableComponentInfo<float>(l, "rotate", COMPONENT_DATA_ROTATE_MASK, a->end, a->end.rotate);
    BuildTableComponentInfo<float>(l, "alpha", COMPONENT_DATA_ROTATE_MASK, a->end, a->end.alpha);
 
    lua_pushstring (l, "duration");
    lua_gettable(l, -2);
    if(!lua_isnil(l, -1)) 
    {
        a->duration = (float)luaL_checknumber(l, -1);
    }
    lua_pop(l, 1);

    a->algorithm = TweenAlgorithm::LINEAR;
    lua_pushstring (l, "algorithm");
    lua_gettable(l, -2);
    if(!lua_isnil(l, -1)) 
    {
        a->algorithm = Tween::getTweenType(luaL_checkstring(l, -1));
    }
    lua_pop(l, 1);

    return a; 
}
//todo: NEED MEMORY TYPE CHECKING LIKE OTHER LUA CLASSES
int LuaAnimate::start(lua_State *l)
{
    Component *component  = (Component *)lua_tointeger(l, 1) ;
    bool loop = (lua_toboolean(l, 2) != 0);
    //todo: make sure it is a component
    Animation *a = (Animation *)create(l);
    a->elapsedTime = 0;
    if(a) {
        a->component = component;
        AnimationChain *chain = manager->start(a, loop, false);
        lua_pushinteger(l, (int)chain);
    }
    return 1; 
}

int LuaAnimate::startChain(lua_State *l)
{
    return 0;
}


int LuaAnimate::destroy(lua_State *l)
{
    Animation *a = (Animation *)luaL_checkinteger(l, 1);
    delete a;

    return 1;
}

