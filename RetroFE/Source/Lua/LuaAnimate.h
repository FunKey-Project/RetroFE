#pragma once

#include "Lua.h"
#include "../Graphics/Animate/AnimationManager.h"

namespace LuaAnimate
{
    void initialize(AnimationManager *m);
    Animation *create(lua_State *l);
    int start(lua_State *l);
    int startChain(lua_State *l);
    int destroy(lua_State *l);
};
