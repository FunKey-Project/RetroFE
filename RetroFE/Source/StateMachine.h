#pragma once

#include "Lua/Lua.h"
#include "Lua/LuaEvent.h"
#include "Control/UserInput.h"

class StateMachine
{
public:
    StateMachine(lua_State *l, LuaEvent *e, UserInput *i);
    void initialize();
    void update(float dt, SDL_Event *e);

private:
    lua_State *luaState_;
    LuaEvent *luaEvent_;
    UserInput *input_;

    enum {
        ON_STARTUP_ENTER,
        ON_STARTUP_UPDATE,
        ON_STARTUP_EXIT,
        ON_STARTUP_EXIT_WAIT,

        ON_COLLECTION_ENTER,
        ON_COLLECTION_UPDATE,
        ON_COLLECTION_EXIT,
        ON_COLLECTION_EXIT_WAIT,

        ON_IDLE_ENTER,
        ON_IDLE_UPDATE,
        ON_IDLE_EXIT,
        ON_IDLE_EXIT_WAIT,

        ON_SCROLL_ENTER,
        ON_SCROLL_UPDATE,
        ON_SCROLL_EXIT,
        ON_SCROLL_EXIT_WAIT

    } state_, nextState_;

};