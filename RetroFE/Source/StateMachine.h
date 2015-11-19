#pragma once

#include "Lua/Lua.h"
#include "Lua/LuaEvent.h"

class StateMachine
{
public:
    StateMachine(lua_State *l, LuaEvent *e);
    void initialize();
    void update(float dt);

private:
    lua_State *luaState_;
    LuaEvent *luaEvent_;

    enum {
        ON_INIT_ENTER,
        ON_INIT_ENTER_WAIT,
        ON_INIT_EXIT,
        ON_INIT_EXIT_WAIT,
    } state_;
};