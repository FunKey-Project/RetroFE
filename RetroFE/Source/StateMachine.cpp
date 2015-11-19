#include "StateMachine.h"

StateMachine::StateMachine(lua_State *l, LuaEvent *e)
: luaState_(l)
, luaEvent_(e)
, state_(ON_INIT_ENTER)
{
}

void StateMachine::update(float dt)
{
    switch(state_) {
        case ON_INIT_ENTER:
            luaEvent_->trigger(luaState_, "onInitEnter");
            state_ = ON_INIT_ENTER_WAIT;
            break;

        case ON_INIT_ENTER_WAIT:
            if(luaEvent_->isComplete(luaState_, "onInitEnter")) {
                state_ = ON_INIT_EXIT;
            }
            break;

        case ON_INIT_EXIT:
            luaEvent_->trigger(luaState_, "onInitExit");
            state_ = ON_INIT_EXIT_WAIT;
            break;

    }
}
