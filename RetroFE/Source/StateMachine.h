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
        ON_STARTUP_ENTER,
        ON_STARTUP_UPDATE,
        ON_STARTUP_EXIT,
        ON_STARTUP_EXIT_WAIT,

        ON_COLLECTION_ENTER,
        ON_COLLECTION_UPDATE,
        ON_COLLECTION_EXIT,
        ON_COLLECTION_EXIT_WAIT,

        ON_STARTUP_EXIT_DONE,
        ON_IDLE_ENTER
    } state_;
};